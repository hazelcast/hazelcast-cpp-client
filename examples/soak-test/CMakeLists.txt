cmake_minimum_required (VERSION 2.6.4)
project(performance_test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS")

if (NOT HAZELCAST_INSTALL_DIR)
    message(FATAL_ERROR "You have to specify HAZELCAST_INSTALL_DIR variable!")
endif()

if (NOT HZ_VERSION)
    message(FATAL_ERROR "You have to specify HZ_VERSION variable!")
endif()

set(HZ_BIT 64)
set(HZ_LIBRARY_NAME HazelcastClient${HZ_VERSION}_${HZ_BIT})
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(PLATFORM Mac)
else ()
    set(PLATFORM ${CMAKE_SYSTEM_NAME})
endif()


message (STATUS "Using Hazelcast installation at ${HAZELCAST_INSTALL_DIR}. PLATFORM:${PLATFORM}, HZ_VERSION:${HZ_VERSION}, HZ_BIT=${HZ_BIT}")

link_libraries(pthread)
set(LIBRARY_DIRECTORY ${HAZELCAST_INSTALL_DIR}/${PLATFORM}_64/hazelcast/lib)
include_directories(${HAZELCAST_INSTALL_DIR}/${PLATFORM}_64/external/include ${HAZELCAST_INSTALL_DIR}/${PLATFORM}_64/hazelcast/include)

add_executable(soakTest SoakTest.cpp)
add_executable(simplePerformanceTest SimplePerformanceTest.cpp)

ADD_LIBRARY(${HZ_LIBRARY_NAME} STATIC IMPORTED)
SET_TARGET_PROPERTIES(${HZ_LIBRARY_NAME} PROPERTIES IMPORTED_LOCATION ${LIBRARY_DIRECTORY}/lib${HZ_LIBRARY_NAME}.dylib)
target_link_libraries(soakTest ${HZ_LIBRARY_NAME})
target_link_libraries(simplePerformanceTest ${HZ_LIBRARY_NAME})

