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

target_link_libraries(
    client_test
    PRIVATE hazelcast-cpp-client ${GTEST_TARGET}
)

# First, try to find Thrift using find_package
# The Thrift library itself doesn't support CMake for users
# but package managers like Vcpkg and Conan provide package config files.
find_package(Thrift QUIET)
if (Thrift_FOUND)
    # If found, the necessary include and library directories
    # will be inferred from the target "thrift::thrift".
    target_link_libraries(client_test PRIVATE thrift::thrift)
else()
    # If find_package fails, we fallback to use the thrift library on
    # system's default paths. (e.g. /usr/lib64, /usr/local/lib)
    # This library is likely to be installed using the system's
    # package manager (e.g. apt, yum, brew) and its include and library
    # paths will be already in the compiler's search path.
    target_link_libraries(client_test PRIVATE thrift)
endif()
