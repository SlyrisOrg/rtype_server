//
// Created by doom on 24/01/18.
//

#ifndef RTYPE_SERVER_GAME_HPP
#define RTYPE_SERVER_GAME_HPP

#include <protocol/Protocol.hpp>
#include <meta/List.hpp>

namespace game
{
    struct Welcome
    {
        static constexpr auto serializableFields() noexcept
        {
            return meta::makeMap();
        }

        reflect_class(Welcome);
    };

    struct MatchStarted
    {
        static constexpr auto serializableFields() noexcept
        {
            return meta::makeMap();
        }

        reflect_class(MatchStarted);
    };

    struct Authenticate
    {
        std::string authToken;

        static constexpr auto serializableFields() noexcept
        {
            return meta::makeMap(reflect_member(&Authenticate::authToken));
        }

        reflect_class(Authenticate);
    };

    using Packets = meta::TypeList<Welcome, MatchStarted, Authenticate>;
}

#endif //RTYPE_SERVER_GAME_HPP
