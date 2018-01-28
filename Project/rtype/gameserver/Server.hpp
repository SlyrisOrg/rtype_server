//
// Created by doom on 23/01/18.
//

#ifndef RTYPE_SERVER_SERVER_HPP
#define RTYPE_SERVER_SERVER_HPP

#include <vector>
#include <thread>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include <utils/Enums.hpp>
#include <log/Logger.hpp>
#include <api/API.hpp>
#include <gameserver/ServerIOThread.hpp>
#include <gameserver/ConfigManager.hpp>
#include <entity/ECS.hpp>
#include <entity/GameFactory.hpp>

namespace rtype
{
    ENUM(Mode,
         Solo,
         Duo,
         Trio,
         Quatuor);

    class GameServer
    {
    public:
        GameServer(unsigned short port, Mode mode, const std::vector<std::string> &authToks) noexcept :
            _port(port), _mode(mode), _authToks(authToks.begin(), authToks.end())
        {
        }

    private:
        void _handleAuth(const game::Authenticate &auth, size_t senderID) noexcept
        {
            if (_authToks.count(auth.authToken) > 0) {
                rtype::Player player;
                player.authToken = auth.authToken;

                std::error_code ec;
                rtype::API::getData(player, ec).wait();

                if (!ec) {
                    game::CreatePlayer cp;
                    cp.nickName = player.nickName;
                    cp.factionName = Faction::toString(player.faction);
                    cp.pos = sf::Vector2f(200, 200 * _authToks.size());
                    GameFactory::createPlayerSpaceShip(_cfg.factionToBounds(cp.factionName), cp.pos);
                    _ioThread.broadcastPacket(cp);
                    _idToNickname.emplace(senderID, cp.nickName);
                    _authToks.erase(auth.authToken);
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
                    _handleAuth(auth, peerAndPacket.id);
                }, [this, &peerAndPacket](auto &&v) {
                    using Decayed = std::decay_t<decltype(v)>;
                    if constexpr (!std::is_same_v<std::monostate, Decayed>) {
                        _log(logging::Debug) << "Got unexpected packet " << Decayed::className()
                                             << " from player " << peerAndPacket.id << std::endl;
                    }
                });

                std::visit(visitor, peerAndPacket.packet);
            }
        }

        void _update(const sf::Time &elapsed) noexcept
        {
            _handleReceivedPackets();
        }

    public:
        void start() noexcept
        {
            _log(logging::Info) << "Starting game with mode " << _mode.toString() << std::endl;
            _log(logging::Info) << "Using port " << _port << std::endl;
            if (!_cfg.loadConfig()) {
                _log(logging::Error) << "Unable to load configuration files" << std::endl;
                return;
            }
            _log(logging::Info) << "Successfully loaded configuration files" << std::endl;
            GameFactory::setEntityManager(&_ettMgr);
            _ioThread.run(_port);

            sf::Clock clock;
            sf::Time timeSinceLastUpdate;
            const sf::Time TimePerFrame = sf::seconds(1.f / 60.f);
            while (!_ioThread.stopped()) {
                _update(timeSinceLastUpdate);
                timeSinceLastUpdate = clock.restart();

                if (auto rest = TimePerFrame.asMilliseconds() - clock.getElapsedTime().asMilliseconds(); rest > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(rest));
                }
            }
        }

    private:
        ServerIOThread _ioThread;
        std::unordered_map<size_t, std::string> _idToNickname;
        ConfigManager _cfg;
        EntityManager _ettMgr;

        unsigned short _port;
        Mode _mode;
        std::set<std::string> _authToks;
        logging::Logger _log{"game", logging::Debug};
    };
}

#endif //RTYPE_SERVER_SERVER_HPP
