//
// Created by szterg_r on 22/10/2017.
//

#include <error/RTypeError.hpp>

const detail::RtypeErrorCategory &login_error_category() noexcept
{
    static detail::RtypeErrorCategory c;
    return c;
}