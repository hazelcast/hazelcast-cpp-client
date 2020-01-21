project(HazelcastClientTest)

find_package(Thrift QUIET)
IF (NOT Thrift_FOUND)
    set(THRIFT_INCLUDE_DIRS /usr/local/opt/thrift/include)
    set(THRIFT_LIBRARIES /usr/local/opt/thrift/lib)
ENDIF(NOT Thrift_FOUND)

message(STATUS "Using THRIFT_INCLUDE_DIRS: ${THRIFT_INCLUDE_DIRS}")
message(STATUS "Using THRIFT_LIBRARIES: ${THRIFT_LIBRARIES}")

include_directories(${THRIFT_INCLUDE_DIRS})

FILE(GLOB_RECURSE HZ_TEST_SOURCES "./*cpp")
FILE(GLOB_RECURSE HZ_TEST_HEADERS "./*h")

set(GOOGLETEST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/hazelcast/test/googletest/googletest/include)

SET(HZ_EXE_NAME clientTest_${HZ_LIB_TYPE}_${HZ_BIT})

add_executable(${HZ_EXE_NAME} ${HZ_TEST_SOURCES} ${HZ_TEST_HEADERS})

add_dependencies(${HZ_EXE_NAME} gtest)

target_link_libraries(${HZ_EXE_NAME} ${HZ_LIB_NAME} thrift gtest)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
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

include_directories(${CMAKE_SOURCE_DIR}/hazelcast/include ${CMAKE_SOURCE_DIR}/hazelcast/test/src ${GOOGLETEST_INCLUDE_DIR} ${BOOST_HOME})

add_definitions(-DHAZELCAST_VERSION=4.0-SNAPSHOT)
