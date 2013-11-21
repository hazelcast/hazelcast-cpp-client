cmake_minimum_required (VERSION 2.6)
project (hz_cppClient_test)

FILE(GLOB_RECURSE HZ_TEST_SOURCES "./*cpp")

include_directories(${CMAKE_SOURCE_DIR}/hazelcast/include ${CMAKE_SOURCE_DIR}/hazelcast/test)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/java/)
add_executable(clientTest ${HZ_TEST_SOURCES})
target_link_libraries(clientTest hazelcastClientStatic ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
#target_link_libraries(clientTest hazelcastClientShared ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
