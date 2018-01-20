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
                                 << playerData->info.getNickname() << "'" << std::endl;
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
                                     << playerData->info.getNickname() << std::endl;

                static const auto visitor = meta::makeVisitor([&playerData](const matchmaking::Authenticate &auth) {
                    playerData->info.setAuthToken(auth.authToken);
                    std::error_code errorCode;
                    rtype::API::getData(playerData->info, errorCode).wait();
                    return !errorCode;
                }, [this, &playerData](const matchmaking::QueueJoin &queuej) {
                    if (playerData->info.getAuthToken().empty()) {
                        return false;
                    }
                    _matchMaker.addPlayer(&playerData->info, queuej.mode);
                    if (_matchMaker.canStartMatch(queuej.mode)) {
                        startMatch(queuej.mode);
                    }
                    return true;
                }, [this, &playerData](const matchmaking::QueueLeave &) {
                    _matchMaker.remove(&playerData->info);
                    return true;
                }, [](const std::monostate &) {
                    return false;
                });

                if (!std::visit(visitor, packet)) {
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
