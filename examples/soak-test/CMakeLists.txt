cmake_minimum_required (VERSION 3.12.4)
project(soak_test)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

include(FindBoost)

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

find_package(Boost REQUIRED COMPONENTS thread chrono)
include_directories(${Boost_INCLUDE_DIRS})
link_libraries(Boost::thread Boost::chrono)
add_definitions("-DBOOST_THREAD_VERSION=5")
message(STATUS "Using Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Using Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")

message (STATUS "Using Hazelcast installation at ${HAZELCAST_INSTALL_DIR}. PLATFORM:${PLATFORM}, HZ_VERSION:${HZ_VERSION}, HZ_BIT=${HZ_BIT}")

link_libraries(pthread)
set(LIBRARY_DIRECTORY ${HAZELCAST_INSTALL_DIR}/${PLATFORM}_64/hazelcast/lib)
include_directories(${HAZELCAST_INSTALL_DIR}/${PLATFORM}_64/hazelcast/include)

add_executable(soak_test soak_test.cpp)

ADD_LIBRARY(${HZ_LIBRARY_NAME} STATIC IMPORTED)
SET_TARGET_PROPERTIES(${HZ_LIBRARY_NAME} PROPERTIES IMPORTED_LOCATION ${LIBRARY_DIRECTORY}/lib${HZ_LIBRARY_NAME}.a)
target_link_libraries(soak_test ${HZ_LIBRARY_NAME})

