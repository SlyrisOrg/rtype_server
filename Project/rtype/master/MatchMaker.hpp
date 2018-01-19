//
// Created by doom on 18/01/18.
//

#ifndef RTYPE_SERVER_MATCHMAKER_HPP
#define RTYPE_SERVER_MATCHMAKER_HPP

#include <set>
#include <protocol/MatchMaking.hpp>
#include <api/Player.hpp>

namespace matchmaking
{
    struct GameInformation
    {
        Mode gameMode;
        std::vector<rtype::Player *> players;
    };

    class MatchMaker
    {
    public:
        using Player = rtype::Player;

        void addPlayer(Player *p, Mode mode) noexcept
        {
            _queues[mode].insert(p);
        }

        void remove(Player *p) noexcept
        {
            for (auto &curQueue : _queues) {
                curQueue.erase(p);
            }
        }

        bool canStartMatch(Mode mode) const noexcept
        {
            return _queues[mode].size() == static_cast<size_t>(mode) + 1;
        }

        GameInformation extractGame(Mode mode) noexcept
        {
            GameInformation ret;

            ret.gameMode = mode;

            for (size_t nb = static_cast<size_t>(mode) + 1; nb; nb--) {
                auto it = _queues[mode].begin();
                ret.players.push_back(*it);
                _queues[mode].erase(it);
            }
            return ret;
        }

    private:
        template <typename T>
        using Queue = std::set<T>;

        std::array<Queue<rtype::Player *>, Mode::NbModes> _queues;
    };
}

#endif //RTYPE_SERVER_MATCHMAKER_HPP
