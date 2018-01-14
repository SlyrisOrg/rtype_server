set(SOURCES tcppacketreader-test.cpp)

add_test_core(tcppacketreaderTest tcppacketreader-test ${SOURCES})
target_include_directories(tcppacketreader-test PRIVATE ${CMAKE_SOURCE_DIR}/Project/rtype)
target_link_libraries(tcppacketreader-test Boost::system Boost::thread Boost::boost core-utils core-meta core-serialization)