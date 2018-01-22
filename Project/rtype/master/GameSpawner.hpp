//
// Created by doom on 22/01/18.
//

#ifndef RTYPE_SERVER_GAMESPAWNER_HPP
#define RTYPE_SERVER_GAMESPAWNER_HPP

#include <sstream>
#include <boost/process.hpp>
#include <master/MatchMaker.hpp>

namespace bp = boost::process;

namespace rtype::master
{
    class GameSpawner
    {
    public:
        GameSpawner(unsigned short port) noexcept : _port(port)
        {
        }

        void spawnGame(const matchmaking::GameInformation &game) noexcept
        {
            std::ostringstream oss;

            oss << "./start_game";
            for (const auto &cur : game.players) {
                oss << " --player=" << cur->authToken;
            }
            oss << "--mode=" << game.gameMode;
            oss << "--port=" << _port++;

            bp::child gameProc(oss.str());
            gameProc.detach();
        }

        unsigned short getNextPort() const noexcept
        {
            return _port;
        }

    private:
        unsigned short _port;
    };
}

#endif //RTYPE_SERVER_GAMESPAWNER_HPP
