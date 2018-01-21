//
// Created by doom on 18/01/18.
//

#ifndef RTYPE_SERVER_PLAYERPOOL_HPP
#define RTYPE_SERVER_PLAYERPOOL_HPP

#include <tuple>
#include <set>
#include <meta/Visitor.hpp>
#include <log/Logger.hpp>
#include <api/Player.hpp>
#include <protocol/MatchMaking.hpp>
#include <netproto/TCPProtocolConnection.hpp>
#include <master/MatchMaker.hpp>
#include <api/API.hpp>

namespace rtype::master
{
    class PlayerPool
    {
    public:
        void addPlayer(tcp::socket &&socket) noexcept
        {
            _log(logging::Debug) << "Adding socket from " << socket.remote_endpoint().address()
                                 << " to player pool" << std::endl;
            auto pair = _players.insert(new PlayerData{std::move(socket)});
            auto &playerData = *pair.first;
            readPacket(playerData);
        }

    private:
        using PacketReader = TCPProtocolConnection<matchmaking::Packets>;
        using Packet = typename PacketReader::Packet;

        struct PlayerData
        {
            explicit PlayerData(tcp::socket &&socket) noexcept : reader(std::move(socket))
            {
            }

            rtype::Player info;
            PacketReader reader;
        };

        void readPacket(PlayerData *playerData) noexcept
        {
            playerData->reader.asyncRead(boost::bind(&PlayerPool::handlePlayerPacket, this,
                                                     asio::placeholders::error, playerData));
        }

        void disconnectPlayer(PlayerData *playerData) noexcept
        {
            _log(logging::Debug) << "Closing connection to player '"
                                 << playerData->info.nickName << "'" << std::endl;
            _matchMaker.remove(&playerData->info);
            _players.erase(playerData);
            delete playerData;
        }

        static auto packetToString(const Packet &p) noexcept
        {
            static const auto v = meta::makeVisitor([](const matchmaking::Authenticate &) {
                return "matchmaking::Authenticate";
            }, [](const matchmaking::QueueJoin &) {
                return "matchmaking::QueueJoin";
            }, [](const matchmaking::QueueLeave &) {
                return "matchmaking::QueueLeave";
            }, [](auto &&) {
                return "invalid packet";
            });

            return std::visit(v, p);
        }

        void startMatch(matchmaking::Mode mode) noexcept
        {
            _log(logging::Debug) << "Starting game for mode " << mode << std::endl;
            matchmaking::GameInformation game = _matchMaker.extractGame(mode);
            auto logScope = _log(logging::Debug);
            logScope << "The players are: ";
            for (const auto &curPlayer : game.players) {
                logScope << curPlayer->nickName << " ";
            }
            logScope << std::endl;
        }

        void handlePlayerPacket(const boost::system::error_code &ec, PlayerData *playerData)
        {
            if (ec) {
                disconnectPlayer(playerData);
                return;
            }

            if (playerData->reader.available() > 0) {
                auto packet = playerData->reader.pop();

                _log(logging::Debug) << "Got packet " << packetToString(packet) << " from "
                                     << playerData->info.nickName << std::endl;

                bool success = false;

                if (std::holds_alternative<matchmaking::Authenticate>(packet)) {
                    const auto &auth = std::get<matchmaking::Authenticate>(packet);
                    playerData->info.authToken = auth.authToken;
                    std::error_code errorCode;
                    rtype::API::getData(playerData->info, errorCode).wait();
                    success =  !errorCode;
                } else if (std::holds_alternative<matchmaking::QueueJoin>(packet)) {
                    const auto &queuej = std::get<matchmaking::QueueJoin>(packet);
                    if (!playerData->info.authToken.empty()) {
                        _matchMaker.addPlayer(&playerData->info, queuej.mode);
                        boost::system::error_code err;
                        playerData->reader.write(matchmaking::QueueStarted{}, err);
                        if (!err) {
                            if (_matchMaker.canStartMatch(queuej.mode)) {
                                startMatch(queuej.mode);
                            }
                            success = true;
                        }
                    }
                } else if (std::holds_alternative<matchmaking::QueueLeave>(packet)) {
                    _matchMaker.remove(&playerData->info);
                }

                if (!success) {
                    disconnectPlayer(playerData);
                } else {
                    readPacket(playerData);
                }
            }
        }

        logging::Logger _log{"playerpool", logging::Debug};
        matchmaking::MatchMaker _matchMaker;
        std::set<PlayerData *> _players;
    };
}

#endif //RTYPE_SERVER_PLAYERPOOL_HPP
