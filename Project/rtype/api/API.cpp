//
// Created by roman sztergbaum on 15/01/2018.
//

#include <api/API.hpp>

namespace rtype
{
    lg::Logger API::_log = lg::Logger{"RType-API", lg::Debug};
    web::http::http_request API::_request = web::http::http_request();
    const std::string API::_urlAPI = "http://rtype.slyris.eu/api/";
}
