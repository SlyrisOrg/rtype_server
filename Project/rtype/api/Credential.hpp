//
// Created by roman sztergbaum on 15/01/2018.
//

#ifndef RTYPE_SERVER_CREDENTIAL_HPP
#define RTYPE_SERVER_CREDENTIAL_HPP

#include <string>
#include <array>

namespace rtype
{
    class Credential
    {
    public:
        enum Field
        {
            Username,
            Password
        };

    public:
        Credential(std::string username, std::string password) noexcept :
            _credentials{{std::move(username), std::move(password)}}
        {
        }

        const std::string& operator[](Field field) const noexcept
        {
            return _credentials[field];
        }
    private:
        std::array<std::string, 2> _credentials;
    };
}

#endif //RTYPE_SERVER_CREDENTIAL_HPP
