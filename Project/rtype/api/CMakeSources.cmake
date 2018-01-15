set(API ${CMAKE_SOURCE_DIR}/Project/rtype/api/)

set(RTYPE_API_HEADERS
        ${API}/API.hpp
        ${API}/Credential.hpp
        ${API}/Player.hpp
        PARENT_SCOPE)

set(RTYPE_API_SOURCES
        ${API}/API.cpp
        PARENT_SCOPE)
