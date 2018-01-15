//
// Created by roman sztergbaum on 15/01/2018.
//

#ifndef RTYPE_SERVER_PLAYER_HPP
#define RTYPE_SERVER_PLAYER_HPP

#include <string>
#include <utils/Enums.hpp>

namespace rtype
{
    ENUM(Faction,
        Bheet,
        Kooy,
        Maul);

    class Player
    {
    public:
        using FactionT = Faction::EnumType;

        void setAuthToken(std::string authToken) noexcept
        {
            _authToken = std::move(authToken);
        }

        const std::string &getAuthToken() const noexcept
        {
            return _authToken;
        }

        void setPseudo(std::string pseudo) noexcept
        {
            _nickname = std::move(pseudo);
        }

        const std::string &getPseudo() const noexcept
        {
            return _nickname;
        }

        void setXP(float xp) noexcept
        {
            _xp = xp;
        }

        float getXP() const noexcept
        {
            return _xp;
        }

        void setLvl(unsigned int lvl) noexcept
        {
            _lvl = lvl;
        }

        unsigned int getLvl() const noexcept
        {
            return _lvl;
        }

        void setGold(unsigned int gold) noexcept
        {
            _gold = gold;
        }

        void setFaction(FactionT faction) noexcept
        {
            _faction = faction;
        }

        Faction getFaction() const noexcept
        {
            return _faction;
        }

        std::string getFactionStr() const noexcept
        {
            return _faction.toString();
        }

    private:
        std::string _authToken;
        std::string _nickname;
        float _xp;
        unsigned int _gold;
        unsigned int _lvl;
        Faction _faction;
    };
}

#endif //RTYPE_SERVER_PLAYER_HPP
