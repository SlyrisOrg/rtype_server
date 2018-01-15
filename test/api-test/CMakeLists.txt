set(SOURCES api-test.cpp
        ${CMAKE_SOURCE_DIR}/Project/rtype/api/API.cpp
        ${CMAKE_SOURCE_DIR}/Project/rtype/error/RTypeError.cpp)

message(sources -> "${SOURCES}")
add_test_core(apiTest api-test "${SOURCES}")
target_include_directories(api-test PRIVATE ${CMAKE_SOURCE_DIR}/Project/rtype)
if (CLANG)
    target_compile_options(api-test PUBLIC "-Wno-unused-lambda-capture")
endif()
target_link_libraries(api-test Boost::system Boost::thread Boost::boost OpenSSL::SSL cpprestsdk::cpprest core-utils core-log core-meta core-serialization)