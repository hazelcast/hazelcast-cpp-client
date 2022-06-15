cmake_minimum_required (VERSION 3.10)

FILE(GLOB_RECURSE HZ_TEST_SOURCES "./*cpp")
FILE(GLOB_RECURSE HZ_TEST_HEADERS "./*h")

add_executable(client_test ${HZ_TEST_SOURCES} ${HZ_TEST_HEADERS})

target_link_libraries(client_test PRIVATE hazelcast-cpp-client ${GTEST_TARGET})

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(Thrift QUIET)
    IF (NOT Thrift_FOUND)
        set(THRIFT_INCLUDE_DIR /usr/include)
        set(THRIFT_LIB_DIR /usr/lib64)
    ENDIF(NOT Thrift_FOUND)
    message(STATUS "Using THRIFT_INCLUDE_DIR: ${THRIFT_INCLUDE_DIR}")
    message(STATUS "Using THRIFT_LIB_DIR: ${THRIFT_LIB_DIR}")
    include_directories(${THRIFT_INCLUDE_DIR})
    target_link_directories(client_test PRIVATE ${THRIFT_LIB_DIR})
    target_link_libraries(client_test PRIVATE thrift)

    set_target_properties(client_test PROPERTIES COMPILE_FLAGS "${HZ_BIT_FLAG} ${HZ_CODE_COVERAGE_COMPILE_FLAGS} ${HZ_VALGRIND_COMPILE_FLAGS}" LINK_FLAGS "${HZ_BIT_FLAG} ${HZ_CODE_COVERAGE_LINK_FLAGS} ${HZ_VALGRIND_LINK_FLAGS} -Wl,--no-as-needed -ldl")
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_package(Thrift QUIET)
    IF (Thrift_FOUND)
        include_directories(${THRIFT_INCLUDE_DIR})
        link_directories(${THRIFT_LIBRARIES})
        target_link_libraries(client_test PRIVATE ${THRIFT_LIBRARIES})
        message(STATUS "Using THRIFT_INCLUDE_DIR: ${THRIFT_INCLUDE_DIR}")
        message(STATUS "Using THRIFT_LIBRARIES: ${THRIFT_LIBRARIES}")
    ELSE (Thrift_FOUND)
        set(THRIFT_INCLUDE_DIR /usr/local/opt/thrift/include)
        set(THRIFT_LIB_DIR /usr/local/opt/thrift/lib)
        include_directories(${THRIFT_INCLUDE_DIR})
        target_link_directories(client_test PRIVATE ${THRIFT_LIB_DIR})
        target_link_libraries(client_test PRIVATE thrift)
        message(STATUS "Using THRIFT_INCLUDE_DIR: ${THRIFT_INCLUDE_DIR}")
        message(STATUS "Using THRIFT_LIB_DIR: ${THRIFT_LIB_DIR}")
    ENDIF(Thrift_FOUND)
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    find_package(Thrift REQUIRED)
    set(THRIFT_INCLUDE_DIR ${THRIFT_INCLUDE_DIR}/..)
    message(STATUS "Using THRIFT_INCLUDE_DIR: ${THRIFT_INCLUDE_DIR}")
    message(STATUS "Using THRIFT_LIBRARIES: ${THRIFT_LIBRARIES}")
    include_directories(${THRIFT_INCLUDE_DIR})
    target_link_libraries(client_test PRIVATE ${THRIFT_LIBRARIES})

    set_target_properties(client_test PROPERTIES COMPILE_FLAGS " /EHsc ")
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

target_include_directories(
    client_test
    PRIVATE ${CMAKE_SOURCE_DIR}/hazelcast/test/src
)

if (MSVC)
    target_compile_options(client_test PRIVATE /bigobj)
endif()

target_compile_definitions(client_test PRIVATE HAZELCAST_VERSION="${PROJECT_VERSION}")