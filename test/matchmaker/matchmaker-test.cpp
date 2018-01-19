//
// Created by doom on 18/01/18.
//

#include <gtest/gtest.h>
#include <master/MatchMaker.hpp>

TEST(MatchMaker, Basic)
{
    matchmaking::MatchMaker mm;
    std::vector<rtype::Player> players(3);

    for (auto &cur : players) {
        mm.addPlayer(&cur, matchmaking::Mode::Quatuor);
    }

    ASSERT_FALSE(mm.canStartMatch(matchmaking::Mode::Solo));
    ASSERT_FALSE(mm.canStartMatch(matchmaking::Mode::Duo));
    ASSERT_FALSE(mm.canStartMatch(matchmaking::Mode::Trio));
    ASSERT_FALSE(mm.canStartMatch(matchmaking::Mode::Quatuor));

    rtype::Player p;
    mm.addPlayer(&p, matchmaking::Mode::Trio);

    ASSERT_FALSE(mm.canStartMatch(matchmaking::Mode::Solo));
    ASSERT_FALSE(mm.canStartMatch(matchmaking::Mode::Duo));
    ASSERT_FALSE(mm.canStartMatch(matchmaking::Mode::Trio));
    ASSERT_FALSE(mm.canStartMatch(matchmaking::Mode::Quatuor));

    rtype::Player p2;
    mm.addPlayer(&p2, matchmaking::Mode::Quatuor);

    ASSERT_FALSE(mm.canStartMatch(matchmaking::Mode::Solo));
    ASSERT_FALSE(mm.canStartMatch(matchmaking::Mode::Duo));
    ASSERT_FALSE(mm.canStartMatch(matchmaking::Mode::Trio));
    ASSERT_TRUE(mm.canStartMatch(matchmaking::Mode::Quatuor));
}
