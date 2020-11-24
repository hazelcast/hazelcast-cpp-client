#
# Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# FLAGS
#  -DHZ_LIB_TYPE=[STATIC | SHARED]
#  -DHZ_BIT=[32 | 64]
#  -DHZ_CODE_COVERAGE=ON
#  -DHZ_VALGRIND=ON
#  -DCMAKE_BUILD_TYPE=Debug
#  -DHZ_BUILD_TESTS=ON
#  -DHZ_BUILD_EXAMPLES=ON
#  -DHZ_COMPILE_WITH_SSL=ON
#  -DHZ_DISABLE_LOGGING=ON

cmake_minimum_required (VERSION 3.12.4)

project (HazelcastClient
		VERSION 4.0
		DESCRIPTION "Hazelcast C++ Client"
		HOMEPAGE_URL https://hazelcast.org/clients/cplusplus/
		LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

include(TestBigEndian)
include(FindOpenSSL)
include(FindBoost)

SET(HZ_VERSION 4.0-SNAPSHOT)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
	message(STATUS "Build needs CMAKE_BUILD_TYPE. Setting default as -DCMAKE_BUILD_TYPE=Debug (other option -DCMAKE_BUILD_TYPE=Release)")
	SET(CMAKE_BUILD_TYPE Debug)
endif ()

message(STATUS "Build type is  ${CMAKE_BUILD_TYPE}")

add_definitions(-DHAZELCAST_VERSION="${HZ_VERSION}")

execute_process(COMMAND git show -s --format=%cd OUTPUT_VARIABLE HAZELCAST_GIT_COMMIT_DATE RESULT_VARIABLE GIT_DATE_RESULT)
if (NOT (${GIT_DATE_RESULT} EQUAL 0))
	message(WARNING "Could not execute command git show for obtaining the git commit date. Process exit result:${GIT_DATE_RESULT}")
	set(HAZELCAST_GIT_COMMIT_DATE "NOT_FOUND")
else()
	string(STRIP "${HAZELCAST_GIT_COMMIT_DATE}" HAZELCAST_GIT_COMMIT_DATE)
	if ("${HAZELCAST_GIT_COMMIT_DATE}" STREQUAL "")
		message(WARNING "git show command returned empty commit date.")
		set(HAZELCAST_GIT_COMMIT_DATE "NOT_FOUND")
	endif()
endif ()
message(STATUS "HAZELCAST_GIT_COMMIT_DATE = ${HAZELCAST_GIT_COMMIT_DATE}" )
add_definitions(-DHAZELCAST_GIT_COMMIT_DATE=${HAZELCAST_GIT_COMMIT_DATE})

execute_process(COMMAND git show -s --format=%h OUTPUT_VARIABLE HAZELCAST_GIT_COMMIT_ID RESULT_VARIABLE GIT_COMMIT_ID_RESULT)
if (NOT (${GIT_COMMIT_ID_RESULT} EQUAL 0))
	message(WARNING "Could not execute command git show for obtaining the git commit id. Process exit result:${GIT_DATE_RESULT}")
	set(HAZELCAST_GIT_COMMIT_ID "NOT_FOUND")
else()
	string(STRIP "${HAZELCAST_GIT_COMMIT_ID}" HAZELCAST_GIT_COMMIT_ID)
	if ("${HAZELCAST_GIT_COMMIT_ID}" STREQUAL "")
		message(WARNING "git show command returned empty commit id.")
		set(HAZELCAST_GIT_COMMIT_ID "NOT_FOUND")
	endif()
endif ()
message(STATUS "HAZELCAST_GIT_COMMIT_ID = ${HAZELCAST_GIT_COMMIT_ID}" )
add_definitions(-DHAZELCAST_GIT_COMMIT_ID=${HAZELCAST_GIT_COMMIT_ID})

message(STATUS "Preparing hazelcast client ..... ")

#detect endianness
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if (${IS_BIG_ENDIAN})
	message(STATUS "Current system is a BIG-ENDIAN system")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DHZ_BIG_ENDIAN")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DHZ_BIG_ENDIAN")
else()
	message(STATUS "Current system is a LITTLE-ENDIAN system")
ENDIF (${IS_BIG_ENDIAN})

FILE(GLOB_RECURSE HZ_SOURCES "./hazelcast/src/*cpp")
FILE(GLOB_RECURSE HZ_GENERATED_SOURCES "./hazelcast/generated-sources/src/*cpp")
FILE(GLOB_RECURSE INLINE_SOURCES "./hazelcast/include/*inl")
FILE(GLOB_RECURSE HZ_HEADERS "./hazelcast/include/*h")
FILE(GLOB_RECURSE HZ_GENERATED_HEADERS "./hazelcast/generated-sources/src/*h")

include_directories(${PROJECT_SOURCE_DIR}/hazelcast/include ${PROJECT_SOURCE_DIR}/hazelcast/generated-sources/src)

if (HZ_DISABLE_LOGGING)
	message(STATUS "Logging is disabled.")
	add_definitions(-DHZ_LOGGING_DISABLED)
endif()

if (HZ_COMPILE_WITH_SSL)
	message(STATUS "Building the library with SSL support.")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DHZ_BUILD_WITH_SSL")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DHZ_BUILD_WITH_SSL")

	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		# This is a bug in CMake that causes it to prefer the system version over
		# the one in the specified ROOT folder. see
		# https://stackoverflow.com/questions/58446253/xcode-11-ld-error-your-binary-is-not-an-allowed-client-of-usr-lib-libcrypto-dy
		set(OPENSSL_ROOT_DIR ${OPENSSL_ROOT_DIR} /usr/local/Cellar/openssl@1.1/1.1.1g/)
		set(OPENSSL_INCLUDE_DIR /usr/local/Cellar/openssl@1.1/1.1.1g/include CACHE FILEPATH "" FORCE)
		set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib CACHE FILEPATH "" FORCE)
		set(OPENSSL_SSL_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libssl.dylib CACHE FILEPATH "" FORCE)
	endif()

	find_package(OpenSSL REQUIRED)
	include_directories(${OPENSSL_INCLUDE_DIR})
	link_libraries(${OPENSSL_SSL_LIBRARIES} ${OPENSSL_CRYPTO_LIBRARIES})
	message(STATUS "Using Open SSL include directory ${OPENSSL_INCLUDE_DIR}")
	message(STATUS "Using Open SSL link libraries ${OPENSSL_SSL_LIBRARIES} ${OPENSSL_CRYPTO_LIBRARIES}")
ELSE ()
	message(STATUS "Building the library without SSL.")
ENDIF()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	if (${CMAKE_BUILD_TYPE} MATCHES "Release")
		set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
		set(Boost_USE_MULTITHREADED      ON)
		set(Boost_USE_STATIC_RUNTIME     OFF)
		set(Boost_USE_DEBUG_RUNTIME      OFF)

		set(Boost_USE_DEBUG_LIBS         OFF)  # ignore debug libs and
		set(Boost_USE_RELEASE_LIBS       ON)   # only find release libs
	else()
		set(Boost_USE_DEBUG_LIBS         ON)   # only find debug libs
		set(Boost_USE_RELEASE_LIBS       OFF)  # ignore release libs
	endif()
	message(STATUS "Using Boost_USE_DEBUG_LIBS: ${Boost_USE_DEBUG_LIBS}")
	message(STATUS "Using Boost_USE_RELEASE_LIBS: ${Boost_USE_RELEASE_LIBS}")
endif()
find_package(Boost REQUIRED COMPONENTS thread chrono regex)
include_directories(${Boost_INCLUDE_DIRS})
link_libraries(Boost::thread Boost::chrono Boost::regex)
add_definitions("-DBOOST_THREAD_VERSION=5")
message(STATUS "Using Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Using Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")

IF (NOT (${HZ_BIT} MATCHES "32") AND NOT (${HZ_BIT} MATCHES "64"))
    message(STATUS "Build needs HZ_BIT. Setting default as -DHZ_BIT=64 (other option -DHZ_BIT=32)")
    set(HZ_BIT 64)
ENDIF (NOT (${HZ_BIT} MATCHES "32") AND NOT (${HZ_BIT} MATCHES "64"))

IF (NOT (${HZ_LIB_TYPE} MATCHES "STATIC") AND NOT (${HZ_LIB_TYPE} MATCHES "SHARED"))
	message(STATUS "Build needs HZ_LIB_TYPE. Setting default as -DHZ_LIB_TYPE=STATIC (other option -DHZ_LIB_TYPE=SHARED)")
	set(HZ_LIB_TYPE STATIC)
ENDIF(NOT (${HZ_LIB_TYPE} MATCHES "STATIC") AND NOT (${HZ_LIB_TYPE} MATCHES "SHARED") )

message(STATUS "Generating ${HZ_LIB_TYPE} Library")

SET(HZ_LIB_NAME HazelcastClient${HZ_VERSION}_${HZ_BIT})

set(HZ_CODE_COVERAGE_COMPILE_FLAGS " ")
set(HZ_CODE_COVERAGE_LINK_FLAGS " ")

IF(HZ_CODE_COVERAGE)
	message(STATUS "Code coverage is on")
	set(HZ_CODE_COVERAGE_COMPILE_FLAGS " -fprofile-arcs -ftest-coverage -fPIC -O0 ")
	set(HZ_CODE_COVERAGE_LINK_FLAGS " -lgcov -fprofile-arcs -ftest-coverage -fPIC -O0 ")
ENDIF(HZ_CODE_COVERAGE)

set(HZ_VALGRIND_COMPILE_FLAGS " ")
set(HZ_VALGRIND_LINK_FLAGS " ")
IF(HZ_VALGRIND)
	message(STATUS "Valgrind is on")
	set(HZ_VALGRIND_COMPILE_FLAGS " -O0 ")
	set(HZ_VALGRIND_LINK_FLAGS " -O0 ")
ENDIF(HZ_VALGRIND)

message(STATUS "${CMAKE_SYSTEM}")
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	message(STATUS "APPLE ENVIRONMENT DETECTED")

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -Werror -fsanitize=address -fno-omit-frame-pointer")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")

	add_library(${HZ_LIB_NAME} ${HZ_LIB_TYPE} ${HZ_SOURCES} ${INLINE_SOURCES} ${HZ_GENERATED_SOURCES} ${HZ_HEADERS} ${HZ_GENERATED_HEADERS})

	link_libraries(${HZ_LIB_NAME})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	message(STATUS "LINUX ENVIRONMENT DETECTED")

	IF(${HZ_BIT} MATCHES "32")
		SET(HZ_BIT_FLAG " -m32 ")
	ELSE()
		SET(HZ_BIT_FLAG " -m64 ")
	ENDIF(${HZ_BIT} MATCHES "32")

	message(STATUS "${HZ_BIT} Bit")

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pipe -g -Wall -Werror ${HZ_BIT_FLAG} ${HZ_CODE_COVERAGE_COMPILE_FLAGS} ${HZ_VALGRIND_COMPILE_FLAGS}")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pipe -O3 -Wall ${HZ_BIT_FLAG} ${HZ_CODE_COVERAGE_COMPILE_FLAGS} ${HZ_VALGRIND_COMPILE_FLAGS}")

	add_library(${HZ_LIB_NAME} ${HZ_LIB_TYPE} ${HZ_SOURCES} ${INLINE_SOURCES} ${HZ_GENERATED_SOURCES} ${HZ_HEADERS} ${HZ_GENERATED_HEADERS})

	link_libraries(${HZ_LIB_NAME} pthread rt)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	message(STATUS "WINDOWS ENVIRONMENT DETECTED ${CMAKE_GENERATOR} ${CMAKE_BUILD_TYPE} ")
	add_definitions(-D__WIN${HZ_BIT}__ -DWIN${HZ_BIT} -D_WIN${HZ_BIT} -DNOMINMAX)

	IF(${HZ_LIB_TYPE} MATCHES "SHARED")
		add_definitions(-DHAZELCAST_EXPORTS=1)
	ELSE(${HZ_LIB_TYPE} MATCHES "STATIC")
		add_definitions(-DHAZELCAST_EXPORTS=0)
	ENDIF(${HZ_LIB_TYPE} MATCHES "SHARED")

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP /EHsc /bigobj")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP /EHsc /bigobj")

	add_library(${HZ_LIB_NAME} ${HZ_LIB_TYPE} ${HZ_SOURCES} ${INLINE_SOURCES} ${HZ_GENERATED_SOURCES} ${HZ_HEADERS} ${HZ_GENERATED_HEADERS})

	link_libraries(${HZ_LIB_NAME})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_definitions(-DBOOST_NO_AUTO_PTR -DELPP_DISABLE_ASSERT -DELPP_WINSOCK2 -DWIN32_LEAN_AND_MEAN)

IF(HZ_BUILD_TESTS)
	option(INSTALL_GMOCK "Install Googletest's GMock?" OFF)
	option(INSTALL_GTEST "Install Googletest's GTest?" OFF)
	option(BUILD_GTEST "Install Googletest's GMock?" ON)
	option(BUILD_GMOCK "Install Googletest's GTest?" OFF)
	ADD_SUBDIRECTORY(hazelcast/test)
	message(STATUS "Configured to build the tests. BUILD_GTEST=${BUILD_GTEST} BUILD_GMOCK=${BUILD_GMOCK}")
ENDIF(HZ_BUILD_TESTS)

IF(HZ_BUILD_EXAMPLES)
	ADD_SUBDIRECTORY(examples)
	message(STATUS "Configured to build the examples.")
ENDIF(HZ_BUILD_EXAMPLES)
