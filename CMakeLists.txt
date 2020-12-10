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
#  -DHZ_CODE_COVERAGE=ON
#  -DHZ_VALGRIND=ON
#  -DCMAKE_BUILD_TYPE=Debug
#  -DHZ_BUILD_TESTS=ON
#  -DHZ_BUILD_EXAMPLES=ON
#  -DHZ_COMPILE_WITH_SSL=ON
#  -DHZ_DISABLE_LOGGING=ON

cmake_minimum_required(VERSION 3.12.4)

project(HazelcastClient # TODO maybe change name ?
        VERSION 4.0.0
        DESCRIPTION "Hazelcast C++ Client"
        HOMEPAGE_URL https://hazelcast.org/clients/cplusplus/
        LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the default build type if not given
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "CMAKE_BUILD_TYPE was not set, using Release as the default.")
    set(CMAKE_BUILD_TYPE Release)
endif ()

include(FindOpenSSL)
include(FindBoost)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# TODO maybe put this in another CMake module.
function(get_commit_date_and_id date id)
    execute_process(COMMAND git show -s --format=%cd OUTPUT_VARIABLE date_out RESULT_VARIABLE date_ret_val)
    execute_process(COMMAND git show -s --format=%h OUTPUT_VARIABLE id_out RESULT_VARIABLE id_ret_val)

    if (date_ret_val)
        message(WARNING "Could not execute command git show for obtaining the git commit date. Process exited with ${date_ret_val}.")
        set(date_out NOT_FOUND)
    else()
        string(STRIP ${date_out} date_out)
        if (NOT date_out)
            message(WARNING "git show command returned empty commit date.")
            set(date_out NOT_FOUND)
        endif()
    endif()

    if (id_ret_val)
        message(WARNING "Could not execute command git show for obtaining the git commit id. Process exited with ${id_ret_val}.")
        set(id_out NOT_FOUND)
    else()
        string(STRIP ${id_out} id_out)
        if (NOT id_out)
            message(WARNING "git show command returned empty commit id.")
            set(id_out NOT_FOUND)
        endif()
    endif()

    set(${date} ${date_out} PARENT_SCOPE)
    set(${id} ${id_out} PARENT_SCOPE)
endfunction()

get_commit_date_and_id(HAZELCAST_GIT_COMMIT_DATE HAZELCAST_GIT_COMMIT_ID)
message(STATUS "HAZELCAST_GIT_COMMIT_DATE = ${HAZELCAST_GIT_COMMIT_DATE}")
message(STATUS "HAZELCAST_GIT_COMMIT_ID = ${HAZELCAST_GIT_COMMIT_ID}")

FILE(GLOB_RECURSE HZ_SOURCES "./hazelcast/src/*cpp")
FILE(GLOB_RECURSE HZ_GENERATED_SOURCES "./hazelcast/generated-sources/src/*cpp")
FILE(GLOB_RECURSE HZ_HEADERS "./hazelcast/include/*h")
FILE(GLOB_RECURSE HZ_GENERATED_HEADERS "./hazelcast/generated-sources/src/*h")


if (NOT (${HZ_LIB_TYPE} MATCHES "STATIC") AND NOT (${HZ_LIB_TYPE} MATCHES "SHARED"))  # TODO provide option to build both? maybe not options at all
    # TODO improve message
    message(STATUS "Build needs HZ_LIB_TYPE. Setting default as -DHZ_LIB_TYPE=STATIC (other option -DHZ_LIB_TYPE=SHARED)")
    set(HZ_LIB_TYPE STATIC)
endif ()
message(STATUS "Generating ${HZ_LIB_TYPE} Library")

add_library(
    HazelcastClient # TODO Maybe HazelcastClientCxx or Cpp in case there is a C library some day?
    ${HZ_LIB_TYPE}
    ${HZ_SOURCES} ${HZ_GENERATED_SOURCES} ${HZ_HEADERS} ${HZ_GENERATED_HEADERS}
)

if (MSVC)
    target_compile_options(HazelcastClient PRIVATE /bigobj)
    target_compile_definitions(HazelcastClient PUBLIC NOMINMAX) # TODO remove this and use (min)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(HazelcastClient PUBLIC pthread rt) # TODO make sure these are actually required
endif ()

if (HZ_COMPILE_WITH_SSL)
    message(STATUS "Building the library with SSL support.")

    target_compile_definitions(
        HazelcastClient
        PUBLIC HZ_BUILD_WITH_SSL
    )

    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        # This is a bug in CMake that causes it to prefer the system version over
        # the one in the specified ROOT folder. see
        # https://stackoverflow.com/questions/58446253/xcode-11-ld-error-your-binary-is-not-an-allowed-client-of-usr-lib-libcrypto-dy
        # TODO search for a better solution to this
        set(OPENSSL_ROOT_DIR /usr/local/Cellar/openssl@1.1/1.1.1h/)
        set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include CACHE FILEPATH "" FORCE)
        set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib CACHE FILEPATH "" FORCE)
        set(OPENSSL_SSL_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libssl.dylib CACHE FILEPATH "" FORCE)
    endif ()

    find_package(OpenSSL REQUIRED)
    target_link_libraries(HazelcastClient PUBLIC OpenSSL::SSL OpenSSL::Crypto) # TODO are these public or private, probably public because asio depends on openssl
else ()
    message(STATUS "Building the library without SSL.")
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    if (${CMAKE_BUILD_TYPE} MATCHES "Release")
        set(Boost_USE_STATIC_LIBS ON)  # only find static libs
        set(Boost_USE_MULTITHREADED ON)
        set(Boost_USE_STATIC_RUNTIME OFF)
        set(Boost_USE_DEBUG_RUNTIME OFF)

        set(Boost_USE_DEBUG_LIBS OFF)  # ignore debug libs and
        set(Boost_USE_RELEASE_LIBS ON)   # only find release libs
    else ()
        set(Boost_USE_DEBUG_LIBS ON)   # only find debug libs
        set(Boost_USE_RELEASE_LIBS OFF)  # ignore release libs
    endif ()
    message(STATUS "Using Boost_USE_DEBUG_LIBS: ${Boost_USE_DEBUG_LIBS}")
    message(STATUS "Using Boost_USE_RELEASE_LIBS: ${Boost_USE_RELEASE_LIBS}")

    # TODO maybe use generate export header from cmake and also think about the flag's public or private
    IF (${HZ_LIB_TYPE} MATCHES "SHARED")
        target_compile_definitions(HazelcastClient PUBLIC HAZELCAST_EXPORTS=1)
    ELSE (${HZ_LIB_TYPE} MATCHES "STATIC")
        target_compile_definitions(HazelcastClient PUBLIC HAZELCAST_EXPORTS=0)
    ENDIF (${HZ_LIB_TYPE} MATCHES "SHARED")
endif ()


find_package(Boost REQUIRED COMPONENTS thread chrono)

target_link_libraries(
    HazelcastClient
    PUBLIC Boost::headers Boost::thread Boost::chrono
)

target_compile_definitions(
    HazelcastClient
    PUBLIC BOOST_THREAD_VERSION=5
)

target_include_directories(
    HazelcastClient
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/hazelcast/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/hazelcast/generated-sources/src>
        $<INSTALL_INTERFACE:include/>
)

target_compile_definitions(
    HazelcastClient
    PUBLIC
        HAZELCAST_VERSION="${PROJECT_VERSION}" # TODO make this private and replace usages in tests with the API
    PRIVATE
        HAZELCAST_GIT_COMMIT_DATE=${HAZELCAST_GIT_COMMIT_DATE}
        HAZELCAST_GIT_COMMIT_ID=${HAZELCAST_GIT_COMMIT_ID}
)

if (HZ_DISABLE_LOGGING)
    message(STATUS "Logging is disabled.")
    target_compile_definitions(HazelcastClient PUBLIC HZ_LOGGING_DISABLED)
endif ()

set_target_properties(
    HazelcastClient
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

option(HZ_ENABLE_CLANG_TIDY
        "Enable clang-tidy"
        OFF)

if (HZ_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_PATH NAMES clang-tidy)

    if (CLANG_TIDY_PATH)
        set_target_properties(
            ${HZ_LIB_NAME}
            PROPERTIES
                CXX_CLANG_TIDY ${CLANG_TIDY_PATH}
        )
    else ()
        message(WARNING "clang-tidy not found, ignoring HZ_ENABLE_CLANG_TIDY")
    endif ()
endif ()

target_compile_definitions(
    HazelcastClient
    PUBLIC  # TODO are these public or private
        BOOST_NO_AUTO_PTR WIN32_LEAN_AND_MEAN
)

if (HZ_BUILD_TESTS)
    option(INSTALL_GMOCK "Install Googletest's GMock?" OFF)
    option(INSTALL_GTEST "Install Googletest's GTest?" OFF)
    option(BUILD_GTEST "Install Googletest's GMock?" ON)
    option(BUILD_GMOCK "Install Googletest's GTest?" OFF)
    add_subdirectory(hazelcast/test)
    message(STATUS "Configured to build the tests. BUILD_GTEST=${BUILD_GTEST} BUILD_GMOCK=${BUILD_GMOCK}")
endif()

if (HZ_BUILD_EXAMPLES)
    add_subdirectory(examples)
    message(STATUS "Configured to build the examples.")
endif ()

install(
    TARGETS HazelcastClient
    EXPORT HazelcastClientTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY hazelcast/include/
    TYPE INCLUDE
    FILES_MATCHING PATTERN "*.h"
)

install(
    DIRECTORY hazelcast/generated-sources/src/
    TYPE INCLUDE
    FILES_MATCHING PATTERN "*.h"
)

install(
    EXPORT HazelcastClientTargets
    FILE HazelcastClientTargets.cmake
    NAMESPACE HazelcastClient::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HazelcastClient
)

write_basic_package_version_file(
    HazelcastClientConfigVersion.cmake
    VERSION ${HazelcastClient_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    cmake/HazelcastClientConfig.cmake.in
    HazelcastClientConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HazelcastClient
)

install(
    FILES
        ${CMAKE_BINARY_DIR}/HazelcastClientConfig.cmake
        ${CMAKE_BINARY_DIR}/HazelcastClientConfigVersion.cmake

    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HazelcastClient
)