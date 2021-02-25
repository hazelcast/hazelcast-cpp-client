cmake_minimum_required (VERSION 3.10)

FILE(GLOB_RECURSE TEST_SOURCES "./*cpp")
FILE(GLOB_RECURSE TEST_HEADERS "./*h")

add_executable(client_test ${TEST_SOURCES} ${TEST_HEADERS})

target_compile_definitions(
    client_test
    PRIVATE HAZELCAST_VERSION="${PROJECT_VERSION}"
)

if (MSVC)
    target_compile_options(client_test PRIVATE /bigobj)
endif()

find_package(Thrift 0.13.0 REQUIRED)

target_link_libraries(
    client_test
    PRIVATE hazelcast-cpp-client ${GTEST_TARGET} thrift::thrift
)
