project(HazelcastClientTest)

FILE(GLOB_RECURSE HZ_TEST_SOURCES "./*cpp")
FILE(GLOB_RECURSE HZ_TEST_HEADERS "./*h")

set(GOOGLETEST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/hazelcast/test/googletest/googletest/include)

SET(HZ_EXE_NAME clientTest_${HZ_LIB_TYPE}_${HZ_BIT})

add_executable(${HZ_EXE_NAME} ${HZ_TEST_SOURCES} ${HZ_TEST_HEADERS})

add_dependencies(${HZ_EXE_NAME} gtest)

target_link_libraries(${HZ_EXE_NAME} ${HZ_LIB_NAME} gtest thrift)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    if ("x${BOOST_HOME}" MATCHES "x")
        set (BOOST_HOME ~/Desktop/work/src/boost_1_64_0)
    endif()

    if ("x${THRIFT_HOME}" MATCHES "x")
        set (THRIFT_HOME /usr/local/Cellar/thrift/0.11.0)
    endif()

    set(THRIFT_INCLUDE_DIR ${THRIFT_HOME}/include)
    set(THRIFT_LIB_DIR ${THRIFT_HOME}/lib)
endif()

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    if ("x${BOOST_HOME}" MATCHES "x")
        set (BOOST_HOME /boost_1_64_0)
    endif()

    if ("x${THRIFT_HOME}" MATCHES "x")
        set(THRIFT_INCLUDE_DIR /usr/local/include)
        set(THRIFT_LIB_DIR /usr/local/lib)
    else()
        set(THRIFT_INCLUDE_DIR ${THRIFT_HOME}/include)
        set(THRIFT_LIB_DIR ${THRIFT_HOME}/lib)
    endif()

    set_target_properties(${HZ_EXE_NAME} PROPERTIES COMPILE_FLAGS "${HZ_BIT_FLAG} ${HZ_CODE_COVERAGE_COMPILE_FLAGS} ${HZ_VALGRIND_COMPILE_FLAGS}" LINK_FLAGS "${HZ_BIT_FLAG} ${HZ_CODE_COVERAGE_LINK_FLAGS} ${HZ_VALGRIND_LINK_FLAGS}")
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    IF (${HZ_LIB_TYPE} MATCHES "STATIC")
        message(STATUS "Shall use the STATIC Hazelcast library for the tests.")
        add_definitions(-DHAZELCAST_USE_STATIC)
    ELSE (${HZ_LIB_TYPE} MATCHES "SHARED")
        message(STATUS "Shall use the SHARED Hazelcast library for the tests.")
        add_definitions(-DHAZELCAST_USE_SHARED)
    ENDIF (${HZ_LIB_TYPE} MATCHES "STATIC")

    set_target_properties(${HZ_EXE_NAME} PROPERTIES COMPILE_FLAGS " /EHsc ")
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

message(STATUS "Using BOOST_HOME: ${BOOST_HOME}")
message(STATUS "Using THRIFT_INCLUDE_DIR: ${THRIFT_INCLUDE_DIR}")
message(STATUS "Using THRIFT_LIB_DIR: ${THRIFT_LIB_DIR}")

link_directories(${THRIFT_LIB_DIR})

include_directories(${CMAKE_SOURCE_DIR}/hazelcast/include ${CMAKE_SOURCE_DIR}/hazelcast/test/src ${GOOGLETEST_INCLUDE_DIR} ${THRIFT_INCLUDE_DIR} ${BOOST_HOME})
