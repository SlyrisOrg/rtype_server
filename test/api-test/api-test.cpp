//
// Created by roman sztergbaum on 15/01/2018.
//

#include <thread>
#include <gtest/gtest.h>
#include <api/API.hpp>

using namespace std::chrono_literals;

TEST(APITest, Login)
{
    rtype::Credential cred{"slyris", "romansromans8"};
    rtype::Player player;
    std::error_code ec;
    rtype::API::login(cred, player, ec).wait();
    ASSERT_EQ(ec, RTypeErrorCode::Success);
    ASSERT_TRUE(!player.getAuthToken().empty());
    std::this_thread::sleep_for(1s);
}

TEST(APITest, LoginEmptyUserName)
{
    std::this_thread::sleep_for(1s);
    rtype::Credential cred{"", "romansromans8"};
    rtype::Player player;
    std::error_code ec;
    rtype::API::login(cred, player, ec).wait();
    ASSERT_EQ(ec, RTypeErrorCode::CredentialsEmpty);
    ASSERT_TRUE(player.getAuthToken().empty());
    std::this_thread::sleep_for(1s);
}

TEST(APITest, LoginEmptyUserPassword)
{
    std::this_thread::sleep_for(1s);
    rtype::Credential cred{"slyris", ""};
    rtype::Player player;
    std::error_code ec;
    rtype::API::login(cred, player, ec).wait();
    ASSERT_EQ(ec, RTypeErrorCode::UserPasswordEmpty);
    ASSERT_TRUE(player.getAuthToken().empty());
    std::this_thread::sleep_for(1s);
}

TEST(APITest, LoginBadCredentials)
{
    std::this_thread::sleep_for(1s);
    rtype::Credential cred{"slyris", "wrongpassword"};
    rtype::Player player;
    std::error_code ec;
    rtype::API::login(cred, player, ec).wait();
    ASSERT_EQ(ec, RTypeErrorCode::UserSigningFail);
    ASSERT_TRUE(player.getAuthToken().empty());
    std::this_thread::sleep_for(1s);
}

TEST(APITest, GetData)
{
    std::this_thread::sleep_for(1s);
    rtype::Credential cred{"joriss", "joriss123"};
    rtype::Player player;
    std::error_code ec;
    auto res = rtype::API::login(cred, player, ec).wait();
    if (res == pplx::task_group_status::completed) {
        ASSERT_EQ(ec, RTypeErrorCode::Success);
        ASSERT_TRUE(!player.getAuthToken().empty());
        rtype::API::getData(player, ec).wait();
        ASSERT_EQ(ec, RTypeErrorCode::Success);
        ASSERT_EQ("Bheet", player.getFactionStr());
    }
    std::this_thread::sleep_for(1s);
}