//
// Created by doom on 28/01/18.
//

#ifndef RTYPE_SERVER_CONFIGURATION_HPP
#define RTYPE_SERVER_CONFIGURATION_HPP

#include <utils/Enums.hpp>

namespace rtype
{
    class Configuration
    {
    public:
        ENUM(SoundEffect,
             Click,
             ClickSmooth,
             LoginSuccess,
             ComputerError,
             ChangeFaction,
             Laser4,
             JoinSuccess);
    };
}

#endif //RTYPE_SERVER_CONFIGURATION_HPP
