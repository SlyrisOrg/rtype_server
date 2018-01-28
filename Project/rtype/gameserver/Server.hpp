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
#include <lua/LuaManager.hpp>

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
        bool canActOnEntity(size_t senderID, const std::string &ettName)
        {
            return _clientIDToNickname[senderID] == ettName;
        }

        void _broadcastPlayersInfo() noexcept
        {
            _log(logging::Debug) << "Broadcasting players information" << std::endl;
            for (const auto &cp : _players) {
                _ioThread.broadcastPacket(cp);
            }
            _players.clear();
        }

        void _handleAuth(const game::Authenticate &auth, size_t senderID) noexcept
        {
            if (_authToks.count(auth.authToken) > 0) {
                rtype::Player player;
                player.authToken = auth.authToken;

                std::error_code ec;
                rtype::API::getData(player, ec).wait();

                if (!ec) {
                    _log(logging::Debug) << "Got authentication from " << player.nickName << std::endl;
                    _clientIDToNickname.emplace(senderID, player.nickName);

                    game::CreatePlayer cp;
                    cp.nickName = player.nickName;
                    cp.factionName = Faction::toString(player.faction);
                    cp.pos = sf::Vector2f(200, 200 * _authToks.size());
                    auto id = GameFactory::createPlayerSpaceShip(_cfg.factionToBounds(cp.factionName), cp.pos);
                    _nameToEntityID.emplace(cp.nickName, id);
                    _players.push_back(std::move(cp));
                    _authToks.erase(auth.authToken);
                    if (_players.size() == static_cast<size_t>(_mode) + 1)
                        _broadcastPlayersInfo();
                }
            } else {
                //Disconnect
            }
        }

        void _handleMove(const game::Move &move, size_t senderID) noexcept
        {
            if (canActOnEntity(senderID, move.ettName)) {
                auto id = _nameToEntityID[move.ettName];
                auto &box = _ettMgr[id].getComponent<rtype::components::BoundingBox>();

                int xFact = 0;
                int yFact = 0;
                switch (move.dir) {
                    case game::Move::Up:
                        yFact = -1;
                        break;
                    case game::Move::Down:
                        yFact = 1;
                        break;
                    case game::Move::Left:
                        xFact = -1;
                        break;
                    case game::Move::Right:
                        xFact = 1;
                        break;
                }
                auto pos = box.getPosition();
                pos.x += xFact * move.time * 450;
                pos.y += yFact * move.time * 450;
                box.setPosition(pos);

                game::SetPosition p;
                p.pos = pos;
                p.ettName = move.ettName;
                _ioThread.broadcastPacket(p);
            }
        }

        void _handleReceivedPackets() noexcept
        {
            PeerAndPacket peerAndPacket;

            while (_ioThread.queue().pop(peerAndPacket)) {
                auto visitor = meta::makeVisitor([this, &peerAndPacket](game::Authenticate &auth) {
                    _handleAuth(auth, peerAndPacket.id);
                }, [this, &peerAndPacket](game::Move &move) {
                    _handleMove(move, peerAndPacket.id);
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

        bool _loadConfig() noexcept
        {
            if (!_cfg.loadConfig()) {
                _log(logging::Error) << "Unable to load configuration files" << std::endl;
                return false;
            }
            _log(logging::Info) << "Successfully loaded configuration files" << std::endl;
            return true;
        }

        bool _loadScripts() noexcept
        {
            if (!_luaMgr.loadScript("player.lua")) {
                _log(logging::Error) << "Unable to load scripts" << std::endl;
                return false;
            }
            _log(logging::Info) << "Successfully loaded scripts" << std::endl;
            return true;
        }

    public:
        void start() noexcept
        {
            _log(logging::Info) << "Starting game with mode " << _mode.toString() << std::endl;
            _log(logging::Info) << "Using port " << _port << std::endl;
            if (!_loadConfig() || !_loadScripts())
                return;
            GameFactory::setEntityManager(&_ettMgr);
            _ioThread.run(_port);

            sf::Clock clock;
            sf::Time timeSinceLastUpdate;
            const sf::Time TimePerFrame = sf::seconds(1.f / 60.f);
            while (!_ioThread.stopped()) {
                clock.restart();
                _update(timeSinceLastUpdate);
                timeSinceLastUpdate = clock.getElapsedTime();

                if (auto rest = TimePerFrame.asMilliseconds() - timeSinceLastUpdate.asMilliseconds(); rest > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(rest));
                }
            }
        }

    private:
        ServerIOThread _ioThread;
        std::unordered_map<size_t, std::string> _clientIDToNickname;
        std::unordered_map<std::string, Entity::ID> _nameToEntityID;
        ConfigManager _cfg;
        EntityManager _ettMgr;
        lua::LuaManager _luaMgr{_ettMgr, fs::path("assets/scripts/")};
        std::vector<game::CreatePlayer> _players;

        unsigned short _port;
        Mode _mode;
        std::set<std::string> _authToks;
        logging::Logger _log{"game", logging::Debug};
    };
}

#endif //RTYPE_SERVER_SERVER_HPP
