//
// Created by doom on 22/01/18.
//

#ifndef RTYPE_SERVER_GAMESPAWNER_HPP
#define RTYPE_SERVER_GAMESPAWNER_HPP

#include <master/MatchMaker.hpp>

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
            _port++;
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
