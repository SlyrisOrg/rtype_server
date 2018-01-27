//
// Created by doom on 18/01/18.
//

#ifndef RTYPE_SERVER_MATCHMAKING_HPP
#define RTYPE_SERVER_MATCHMAKING_HPP

#include <protocol/Protocol.hpp>
#include <meta/List.hpp>
#include <api/Player.hpp>

namespace matchmaking
{
    struct Authenticate
    {
        std::string authToken;

        static constexpr auto serializableFields() noexcept
        {
            return meta::makeMap(reflect_member(&Authenticate::authToken));
        }

        reflect_class(Authenticate);
    };

    enum Mode
    {
        Solo,
        Duo,
        Trio,
        Quatuor,
        None,
        NbModes = None,
    };

    struct QueueJoin
    {
        Mode mode;

        static constexpr auto serializableFields() noexcept
        {
            return meta::makeMap(reflect_member(&QueueJoin::mode));
        }

        reflect_class(QueueJoin);
    };

    struct QueueLeave
    {
        static constexpr auto serializableFields() noexcept
        {
            return meta::makeMap();
        }

        reflect_class(QueueLeave);
    };

    struct QueueStarted
    {
        static constexpr auto serializableFields() noexcept
        {
            return meta::makeMap();
        }

        reflect_class(QueueStarted);
    };

    struct MatchFound
    {
        static constexpr auto serializableFields() noexcept
        {
            return meta::makeMap();
        }

        reflect_class(MatchFound);
    };

    struct GameHostInfo
    {
        unsigned short port;

        static constexpr auto serializableFields() noexcept
        {
            return meta::makeMap(reflect_member(&GameHostInfo::port));
        }

        reflect_class(GameHostInfo);
    };

    using PlayerInfo = rtype::Player;

    using Packets = meta::TypeList
        <
            Authenticate,
            QueueJoin,
            QueueLeave,
            QueueStarted,
            MatchFound,
            PlayerInfo,
            GameHostInfo
        >;
}

#endif //RTYPE_SERVER_MATCHMAKING_HPP
