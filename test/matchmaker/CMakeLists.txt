set(SOURCES matchmaker-test.cpp)

add_test_core(matchmakerTest matchmaker-test ${SOURCES})
target_include_directories(matchmaker-test PRIVATE ${CMAKE_SOURCE_DIR}/Project/rtype)
target_link_libraries(matchmaker-test core-utils core-meta core-serialization)
