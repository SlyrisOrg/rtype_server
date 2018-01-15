//
// Created by szterg_r on 22/10/2017.
//

#ifndef RTYPE_LOGINERROR_HPP
#define RTYPE_LOGINERROR_HPP

#include <string>
#include <system_error>

enum class RTypeErrorCode
{
    Success = 0,
    NotFound = 101,
    InternalError = 102,
    InvalidSignature = 103,
    InvalidToken = 104,
    InvalidRequest = 105,
    UserPseudoEmpty = 201,
    UserPseudoBadFormat = 202,
    UserAlreadyTaken = 203,
    UserNameEmpty = 301,
    UserNameBadFormat = 302,
    UserEmailEmpty = 401,
    UserEmailBadFormat = 402,
    UserPasswordEmpty = 501,
    UserPasswordBadFormat = 502,
    UserGetDataFail = 601,
    UserPutDataFail = 701,
    UserPostDataFail = 801,
    UserSigningFail = 901,
    UserSignupFail = 1001,
    CredentialsEmpty = 1101,
    NoInternet = 2001,
};

namespace std // NOLINT
{
    // Tell the C++ 11 STL metaprogramming that enum is registered with the standard error code system
    template <>
    struct is_error_code_enum<RTypeErrorCode> : std::true_type
    {
    };
}

namespace detail
{
    class RtypeErrorCategory : public std::error_category
    {
    public:

        const char *name() const noexcept final
        {
            return "RtypeErrorCategory";
        }

        std::string message(int c) const noexcept final
        {
            switch (static_cast<RTypeErrorCode>(c)) {
                case RTypeErrorCode::Success:
                    return "Success.";
                case RTypeErrorCode::UserSigningFail:
                    return "Username or password is incorrect. Please try logging in again.";
                case RTypeErrorCode::InternalError:
                case RTypeErrorCode::NotFound:
                    return "Internal error occurred, please contact an administrator.";
                case RTypeErrorCode::UserNameEmpty:
                    return "Error username is empty.";
                case RTypeErrorCode::UserPseudoBadFormat:
                    return "Error user pseudo bad format.";
                case RTypeErrorCode::UserPasswordBadFormat:
                    return "Error password bad format.";
                case RTypeErrorCode::UserPasswordEmpty:
                    return "Error password is empty.";
                case RTypeErrorCode::InvalidSignature:
                    return "Error invalid signature.";
                case RTypeErrorCode::InvalidToken:
                    return "Error invalid Token.";
                case RTypeErrorCode::InvalidRequest:
                    return "Error invalid request.";
                case RTypeErrorCode::UserPseudoEmpty:
                case RTypeErrorCode::UserEmailEmpty:
                    return "Error user pseudo is empty.";
                case RTypeErrorCode::UserNameBadFormat:
                case RTypeErrorCode::UserEmailBadFormat:
                    return "Error username bad format.";
                case RTypeErrorCode::UserGetDataFail:
                    return "Error user get data fail";
                case RTypeErrorCode::UserPutDataFail:
                    return "Error user put data fail.";
                case RTypeErrorCode::UserPostDataFail:
                    return "Error user post data fail.";
                case RTypeErrorCode::UserSignupFail:
                    return "Error user signup fail.";
                case RTypeErrorCode::UserAlreadyTaken:
                    return "This nickname is already taken by another user.";
                case RTypeErrorCode::CredentialsEmpty:
                    return "Username or email is missing";
                case RTypeErrorCode::NoInternet:
                    return "Oops, it seems you do not have internet";
            }
            return "";
        }

        std::error_condition default_error_condition(int c) const noexcept final
        {
            switch (static_cast<RTypeErrorCode>(c)) {
                case RTypeErrorCode::UserNameEmpty:
                case RTypeErrorCode::UserPseudoBadFormat:
                case RTypeErrorCode::UserPasswordBadFormat:
                case RTypeErrorCode::UserPasswordEmpty:
                    return std::make_error_condition(std::errc::invalid_argument);
                default:
                    return std::error_condition{c, *this};
            }
        }
    };
}

extern const detail::RtypeErrorCategory &login_error_category() noexcept;

static inline std::error_code make_error_code(RTypeErrorCode e) noexcept
{
    return {static_cast<int>(e), login_error_category()};
}

#endif //RTYPE_LOGINERROR_HPP
