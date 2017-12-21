cmake_minimum_required(VERSION 3.6)
project(rtype_server)

enable_testing()
include(CTest)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(BUILD_CORE_TESTS OFF CACHE BOOL "disable core tests" FORCE)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/vendor/core/cmake-utils/Module)
set(CMAKE_CONFIG_PATH ${CMAKE_SOURCE_DIR}/vendor/core/cmake-utils)
add_subdirectory(vendor/core)

include(${CMAKE_CONFIG_PATH}/include.cmake)
STANDARD_COMPILATION_SETTINGS()

find_package(Boost COMPONENTS system thread REQUIRED)

set(RTYPE_SERVER_SOURCES
        Project/main.cpp)

add_executable(${PROJECT_NAME} ${RTYPE_SERVER_SOURCES})
target_link_libraries(${PROJECT_NAME} Boost::system Boost::thread Boost::boost)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Project)

add_subdirectory(test)