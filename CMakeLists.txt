cmake_minimum_required(VERSION 3.6)
project(rtype_server)

enable_testing()
include(CTest)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(BUILD_CORE_TESTS OFF CACHE BOOL "disable core tests" FORCE)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/vendor/core/cmake-utils/Module)
set(CMAKE_CONFIG_PATH ${CMAKE_SOURCE_DIR}/vendor/core/cmake-utils)

if (WIN32)
    add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        set(BOOST_LIBRARYDIR $ENV{BOOST_ROOT}/debug/lib)
    endif()
endif(WIN32)

find_package(Boost COMPONENTS random regex system filesystem thread program_options REQUIRED)

add_subdirectory(vendor/core)

include(${CMAKE_CONFIG_PATH}/include.cmake)
STANDARD_COMPILATION_SETTINGS()
RTYPE_SERVER_CONFIG()
CPPREST_CONFIG()
find_package(Lua 5.3)

add_subdirectory(Project/rtype/netproto)
add_subdirectory(Project/rtype/protocol)
add_subdirectory(Project/rtype/master)
add_subdirectory(Project/rtype/api)
add_subdirectory(Project/rtype/error)

set(RTYPE_SERVER_SOURCES
        ${RTYPE_PROTO_HEADERS}
        ${RTYPE_MASTER_SOURCES}
        ${RTYPE_NETPROTO_SOURCES}
        ${RTYPE_API_SOURCES}
        ${RTYPE_API_HEADERS}
        ${RTYPE_ERROR_SOURCES}
        ${RTYPE_ERROR_HEADERS}
        Project/main.cpp)

add_executable(${PROJECT_NAME} ${RTYPE_SERVER_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Project/rtype)
target_link_libraries(${PROJECT_NAME} Boost::system Boost::filesystem Boost::thread Boost::program_options Boost::boost
        cpprestsdk::cpprest OpenSSL::SSL core-utils core-log core-meta core-serialization)

add_subdirectory(test)