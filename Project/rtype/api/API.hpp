//
// Created by roman sztergbaum on 15/01/2018.
//

#ifndef RTYPE_SERVER_API_HPP
#define RTYPE_SERVER_API_HPP

#include <log/Logger.hpp>
#undef always_inline
#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include <cpprest/asyncrt_utils.h>
#include <api/Credential.hpp>
#include <api/Player.hpp>
#include <error/RTypeError.hpp>

namespace rtype
{
    namespace lg = logging;

    class API
    {
    private:
        static void _debugRequest(const web::http::http_request &request) noexcept
        {
#if defined(USING_WINDOWS)
            using namespace utility::conversions;
            _log(lg::Debug) << "Http request send ->\n" << utf16_to_utf8(request.to_string()) << std::endl;
#else
            _log(lg::Debug) << "Http request send ->\n" << request.to_string() << std::endl;
#endif
        }

        static void _debugJson(const web::json::value &value) noexcept
        {
            using namespace utility::conversions;
#if defined(USING_WINDOWS)
            _log(lg::Debug) << "Http response receive ->\n" << utf16_to_utf8(value.serialize()) << std::endl;
#else
            _log(lg::Debug) << "Http response receive ->\n" << to_string_t(value.serialize()) << std::endl;
#endif
        }

        static void _setRequest(const web::http::method &method) noexcept
        {
            _request.headers().clear();
            _request.set_method(method);
            _request.headers().add(U("Content-Type"), U("application/json"));
            _request.headers().add(U("X-Hub-Signature"),
                                   U("eAVZepqfXsrSW6LVjTuqb3W3CHsf9mAUa5776cGZ2hLGzztK4PAT5gkJE52h"));
        }

    public:
        static pplx::task<void> login(const Credential &credential, Player &player, std::error_code &ec) noexcept
        {
            const auto url = utility::conversions::to_string_t(_urlAPI + "user/signin");
            web::json::value postData;
            postData[U("name")] = web::json::value::string(
                utility::conversions::to_string_t(credential[Credential::Username]));
            postData[U("password")] = web::json::value::string(
                utility::conversions::to_string_t(credential[Credential::Password]));
            web::http::client::http_client client(url);
            _setRequest(web::http::methods::POST);
            _request.set_body(postData.serialize());
            _debugRequest(_request);

            return client.request(_request).then([&ec, &player](const web::http::http_response &response) {
                if (response.status_code() == web::http::status_codes::OK) {
                    return response.extract_json();
                }
                return pplx::task_from_result(web::json::value());
            }).then([&ec, &player](const pplx::task<web::json::value> &task) {
                try {
                    auto response = task.get();
                    _debugJson(response);
                    web::json::object obj = response.as_object();
                    ec = static_cast<RTypeErrorCode>(obj.at(U("payload")).as_integer());

                    if (!ec) {
                        _log(lg::Info) << ec.message() << std::endl;
#if defined(USING_WINDOWS)
                        const utility::string_t &wtoken = obj.at(U("content")).as_object().at(U("token")).as_string();
                std::string token = utility::conversions::utf16_to_utf8(wtoken);
#else
                        const utility::string_t &token = obj.at(U("content")).as_object().at(U("token")).as_string();
#endif
                        _log(lg::Info) << "Token Session -> " << token << std::endl;

                        player.authToken = "JWT " + token;
                    } else {
                        _log(lg::Warning) << ec.message() << std::endl;
                    }
                }
                catch (const web::http::http_exception &e) {
                    ec = RTypeErrorCode::InternalError;
                    _log(lg::Error) << "Http exception occured : " << e.what() << std::endl;
                }
                catch (const std::exception &e) {
                    ec = RTypeErrorCode::InternalError;
                    _log(lg::Error) << "Unexpected exception occurred : " << e.what() << std::endl;
                }
            });
        };

        static pplx::task<void> getData(Player &player, std::error_code &ec) noexcept
        {
            const auto url = utility::conversions::to_string_t(_urlAPI + "user");
            web::http::client::http_client client(url);
            _setRequest(web::http::methods::GET);
            _request.headers().add(U("Authorization"), utility::conversions::to_string_t(player.authToken));
            _debugRequest(_request);

            return client.request(_request).then([&ec, &player](const web::http::http_response &response) {
                if (response.status_code() == web::http::status_codes::OK) {
                    return response.extract_json();
                }
                return pplx::task_from_result(web::json::value());
            }).then([&ec, &player](const pplx::task<web::json::value> &task) {
                try {
                    auto response = task.get();
                    web::json::object obj = response.as_object();
                    _debugJson(response);
                    ec = static_cast<RTypeErrorCode>(obj.at(U("payload")).as_integer());
                    if (!ec) {
                        _log(lg::Info) << ec.message() << std::endl;
#if defined(USING_WINDOWS)
                        const utility::string_t &wnickname = response.at(U("content")).as_object().at(U("nickname")).as_string();
                        player.nickName = utility::conversions::utf16_to_utf8(wnickname);
#else
                        player.nickName = response.at(U("content")).as_object().at(U("nickname")).as_string();
#endif
                        const auto &profile = response.at(U("content")).as_object().at(U("profile")).as_object();
                        player.xp = static_cast<float>(profile.at(U("experience")).as_double());
                        player.lvl = static_cast<unsigned int>(profile.at(U("level")).as_integer());
                        player.gold = static_cast<unsigned int>(profile.at(U("gold")).as_integer());
                        player.faction = static_cast<Player::FactionT>(profile.at(U("faction")).as_integer());
                    } else {
                        _log(lg::Warning) << ec.message() << std::endl;
                    }
                }
                catch (const web::http::http_exception &e) {
                    ec = RTypeErrorCode::InternalError;
                    _log(lg::Error) << "Http exception occured : " << e.what() << std::endl;
                }
                catch (const std::exception &e) {
                    ec = RTypeErrorCode::InternalError;
                    _log(lg::Error) << "Unexpected exception occurred : " << e.what() << std::endl;
                }
            });
        }

    private:
        static const std::string _urlAPI;
        static lg::Logger _log;
        static web::http::http_request _request;
    };
}

#undef U

#endif //RTYPE_SERVER_API_HPP
