cmake_minimum_required (VERSION 3.12.4)

FILE(GLOB_RECURSE HZ_TEST_SOURCES "./*cpp")
FILE(GLOB_RECURSE HZ_TEST_HEADERS "./*h")

set(GOOGLETEST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/hazelcast/test/googletest/googletest/include)

add_executable(clientTest ${HZ_TEST_SOURCES} ${HZ_TEST_HEADERS})

target_link_libraries(clientTest PRIVATE HazelcastClient gtest)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(Thrift QUIET)
    IF (NOT Thrift_FOUND)
        set(THRIFT_INCLUDE_DIR /usr/include)
        set(THRIFT_LIB_DIR /usr/lib64)
    ENDIF(NOT Thrift_FOUND)
    message(STATUS "Using THRIFT_INCLUDE_DIR: ${THRIFT_INCLUDE_DIR}")
    message(STATUS "Using THRIFT_LIB_DIR: ${THRIFT_LIB_DIR}")
    include_directories(${THRIFT_INCLUDE_DIR})
    target_link_directories(clientTest PRIVATE ${THRIFT_LIB_DIR})
    target_link_libraries(clientTest PRIVATE  thrift)

    set_target_properties(clientTest PROPERTIES COMPILE_FLAGS "${HZ_BIT_FLAG} ${HZ_CODE_COVERAGE_COMPILE_FLAGS} ${HZ_VALGRIND_COMPILE_FLAGS}" LINK_FLAGS "${HZ_BIT_FLAG} ${HZ_CODE_COVERAGE_LINK_FLAGS} ${HZ_VALGRIND_LINK_FLAGS} -Wl,--no-as-needed -ldl")
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_package(Thrift QUIET)
    IF (Thrift_FOUND)
        include_directories(${THRIFT_INCLUDE_DIR})
        link_directories(${THRIFT_LIBRARIES})
        target_link_libraries(clientTest PRIVATE ${THRIFT_LIBRARIES})
        message(STATUS "Using THRIFT_INCLUDE_DIR: ${THRIFT_INCLUDE_DIR}")
        message(STATUS "Using THRIFT_LIBRARIES: ${THRIFT_LIBRARIES}")
    ELSE (Thrift_FOUND)
        set(THRIFT_INCLUDE_DIR /usr/local/opt/thrift/include)
        set(THRIFT_LIB_DIR /usr/local/opt/thrift/lib)
        include_directories(${THRIFT_INCLUDE_DIR})
        target_link_directories(clientTest PRIVATE ${THRIFT_LIB_DIR})
        target_link_libraries(clientTest PRIVATE thrift)
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
    target_link_libraries(clientTest PRIVATE ${THRIFT_LIBRARIES})

    IF (${HZ_LIB_TYPE} MATCHES "STATIC")
        message(STATUS "Shall use the STATIC Hazelcast library for the tests.")
        add_definitions(-DHAZELCAST_USE_STATIC)
    ELSE (${HZ_LIB_TYPE} MATCHES "SHARED")
        message(STATUS "Shall use the SHARED Hazelcast library for the tests.")
        add_definitions(-DHAZELCAST_USE_SHARED)
    ENDIF (${HZ_LIB_TYPE} MATCHES "STATIC")

    set_target_properties(clientTest PROPERTIES COMPILE_FLAGS " /EHsc ")
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

target_include_directories(
    clientTest
    PRIVATE ${CMAKE_SOURCE_DIR}/hazelcast/test/src
)

