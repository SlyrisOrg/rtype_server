//
// Created by doom on 24/01/18.
//

#ifndef RTYPE_SERVER_GAME_HPP
#define RTYPE_SERVER_GAME_HPP

#include <protocol/Protocol.hpp>
#include <meta/List.hpp>

namespace game
{
    struct MatchStarted
    {
        static constexpr auto serializableFields() noexcept
        {
            return meta::makeMap();
        }
    };

    struct Authenticate
    {
        std::string authToken;

        static constexpr auto serializableFields() noexcept
        {
            return meta::makeMap(reflect_member(&Authenticate::authToken));
        }
    };

    using Packets = meta::TypeList<MatchStarted, Authenticate>;
}

#endif //RTYPE_SERVER_GAME_HPP
