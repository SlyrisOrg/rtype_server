//
// Created by doom on 23/01/18.
//

#ifndef RTYPE_SERVER_SERVER_HPP
#define RTYPE_SERVER_SERVER_HPP

#include <vector>
#include <thread>
#include <SFML/Graphics.hpp>
#include <utils/Enums.hpp>
#include <log/Logger.hpp>
#include <api/API.hpp>
#include <gameserver/ServerIOThread.hpp>

namespace rtype
{
    ENUM(Mode,
         Solo,
         Duo,
         Trio,
         Quatuor);

//    ENUM(PeerID,
//         All,
//         One,
//         Two,
//         Three,
//         Four);

    class GameServer
    {
    public:
        GameServer(unsigned short port, Mode mode, const std::vector<std::string> &authToks) noexcept :
            _port(port), _mode(mode), _authToks(authToks.begin(), authToks.end())
        {
        }

        void _handleAuth(const game::Authenticate &auth, const PeerAndPacket &pap) noexcept
        {
            if (_authToks.count(auth.authToken)) {
                rtype::Player player;
                player.authToken = auth.authToken;

                std::error_code ec;
                rtype::API::getData(player, ec).wait();

                if (!ec) {
                    game::CreatePlayer cp;
                    cp.nickName = player.nickName;
                    cp.factionName = Faction::toString(player.faction);
                    cp.pos = sf::Vector2f(200, 200);
                    _ioThread.broadcastPacket(cp);
                }
            } else {
                //Disconnect
            }
        }

        void _handleReceivedPackets() noexcept
        {
            PeerAndPacket peerAndPacket;

            while (_ioThread.queue().pop(peerAndPacket)) {
                auto visitor = meta::makeVisitor([this, &peerAndPacket](game::Authenticate &auth) {
                    _handleAuth(auth, peerAndPacket);
                }, [this, &peerAndPacket](auto &&v) {
                    using Decayed = std::decay_t<decltype(v)>;
                    if constexpr (!std::is_same_v<std::monostate, Decayed>) {
                        _log(logging::Debug) << "Got unexpected apacket "
                                             << Decayed::className()
                                             << " from player " << peerAndPacket.first
                                             << std::endl;
                    }
                });

                std::visit(visitor, peerAndPacket.second);
            }
        }

        void _update(const sf::Time &elapsed) noexcept
        {
            _handleReceivedPackets();
        }

        void start() noexcept
        {
            _log(logging::Info) << "Starting game with mode " << _mode.toString() << std::endl;
            _log(logging::Info) << "Using port " << _port << std::endl;
            _ioThread.run(_port);

            sf::Clock clock;
            sf::Time timeSinceLastUpdate;
            const sf::Time TimePerFrame = sf::seconds(1.f / 60.f);
            while (!_ioThread.stopped()) {
                _update(timeSinceLastUpdate);
                timeSinceLastUpdate = clock.restart();

                if (auto rest = TimePerFrame.asMilliseconds() - clock.getElapsedTime().asMilliseconds();
                rest > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(rest));
                }
            }
        }

    private:
        //TODO: link packet and the person to which we wanna send it
        ServerIOThread _ioThread;

        unsigned short _port;
        Mode _mode;
        std::set<std::string> _authToks;
        logging::Logger _log{"game", logging::Debug};
    };
}

#endif //RTYPE_SERVER_SERVER_HPP
