//
// Created by doom on 18/01/18.
//

#ifndef RTYPE_SERVER_PLAYERPOOL_HPP
#define RTYPE_SERVER_PLAYERPOOL_HPP

#include <set>
#include <unordered_map>
#include <meta/Visitor.hpp>
#include <log/Logger.hpp>
#include <api/Player.hpp>
#include <protocol/MatchMaking.hpp>
#include <netproto/TCPProtocolConnection.hpp>
#include <api/API.hpp>
#include <master/MatchMaker.hpp>
#include <master/GameSpawner.hpp>

namespace rtype::master
{
    class PlayerPool
    {
    public:
        void addPlayer(tcp::socket &&socket) noexcept
        {
            _log(logging::Debug) << "Adding socket from " << socket.remote_endpoint().address()
                                 << " to player pool" << std::endl;
            auto pair = _players.emplace(new PlayerData{std::move(socket)});
            auto &playerData = *pair.first;
            _readPacket(playerData);
        }

    private:
        using Connection = TCPProtocolConnection<matchmaking::Packets>;
        using Packet = typename Connection::Packet;

        struct PlayerData
        {
            explicit PlayerData(tcp::socket &&socket) noexcept : conn(Connection::makeShared(std::move(socket)))
            {
            }

            rtype::Player info;
            boost::shared_ptr<Connection> conn;
        };

        void _readPacket(boost::shared_ptr<PlayerData> playerData) noexcept
        {
            playerData->conn->asyncRead(boost::bind(&PlayerPool::_handlePlayerPacket, this,
                                                    asio::placeholders::error, playerData));
        }

        void _disconnectPlayer(boost::shared_ptr<PlayerData> playerData) noexcept
        {
            auto it = _players.find(playerData);

            if (it != _players.end()) {
                _log(logging::Debug) << "Closing connection to '" << playerData->info.nickName << "'" << std::endl;
                if (playerData->info.hasAuthToken()) {
                    _identifiedPlayers.erase(playerData->info.nickName);
                }
                playerData->conn->socket().close();
                _matchMaker.remove(&playerData->info);
                _players.erase(it);
            }
        }

        static auto _packetToString(const Packet &p) noexcept
        {
            static const auto v = meta::makeVisitor([](auto &&v) -> std::string {
                using Decayed = std::decay_t<decltype(v)>;

                if constexpr (std::is_same_v<Decayed, std::monostate>)
                    return "Invalid";
                else
                    return Decayed::className();
            });

            return std::visit(v, p);
        }

        void _startMatch(matchmaking::Mode mode) noexcept
        {
            _log(logging::Debug) << "Starting game for mode " << mode << std::endl;
            matchmaking::GameInformation game = _matchMaker.extractGame(mode);

            matchmaking::GameHostInfo ghi;
            ghi.port = _gameSpawner.getNextPort();

            auto scopedLog = _log(logging::Debug);
            scopedLog << "The players are: ";
            for (const auto &curPlayer : game.players) {
                scopedLog << curPlayer->nickName << " ";
                auto &conn = _identifiedPlayers[curPlayer->nickName]->conn;
                boost::system::error_code ec;
                conn->write(matchmaking::MatchFound(), ec);
                //TODO: handle potential errors

                for (const auto &curInfo : game.players) {
                    if (curInfo != curPlayer) {
                        auto info = _identifiedPlayers[curInfo->nickName]->info;
                        info.authToken.clear();
                        conn->write(info, ec);
                        //TODO: handle potential errors
                    }
                }
                conn->write(ghi, ec);
            }
            scopedLog << std::endl;

            _gameSpawner.spawnGame(game);
            for (const auto &curPlayer : game.players) {
                _disconnectPlayer(_identifiedPlayers[curPlayer->nickName]);
            }
        }

        void _handlePlayerPacket(const boost::system::error_code &ec, boost::shared_ptr<PlayerData> playerData)
        {
            if (ec) {
                _disconnectPlayer(playerData);
                return;
            }

            if (playerData->conn->available() > 0) {
                auto packet = playerData->conn->pop();

                _log(logging::Debug) << "Got packet " << _packetToString(packet) << " from "
                                     << playerData->info.nickName << std::endl;

                bool success = false;

                if (std::holds_alternative<matchmaking::Authenticate>(packet)) {
                    const auto &auth = std::get<matchmaking::Authenticate>(packet);
                    playerData->info.authToken = auth.authToken;
                    std::error_code errorCode;
                    rtype::API::getData(playerData->info, errorCode).wait();
                    if (!errorCode) {
                        _identifiedPlayers.emplace(playerData->info.nickName, playerData);
                        success = true;
                    }
                } else if (std::holds_alternative<matchmaking::QueueJoin>(packet)) {
                    const auto &queuej = std::get<matchmaking::QueueJoin>(packet);
                    if (!playerData->info.authToken.empty()) {
                        _matchMaker.addPlayer(&playerData->info, queuej.mode);
//                        playerData->conn.write(matchmaking::QueueStarted{}, err);
                        if (_matchMaker.canStartMatch(queuej.mode)) {
                            _startMatch(queuej.mode);
                        }
                        success = true;
                    }
                } else if (std::holds_alternative<matchmaking::QueueLeave>(packet)) {
                    _matchMaker.remove(&playerData->info);
                }

                if (!success) {
                    _disconnectPlayer(playerData);
                } else {
                    _readPacket(playerData);
                }
            }
        }

        logging::Logger _log{"playerpool", logging::Debug};
        matchmaking::MatchMaker _matchMaker;
        std::set<boost::shared_ptr<PlayerData>> _players;
        std::unordered_map<std::string, boost::shared_ptr<PlayerData>> _identifiedPlayers;
        GameSpawner _gameSpawner{31400};
    };
}

#endif //RTYPE_SERVER_PLAYERPOOL_HPP
