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

cmake_minimum_required(VERSION 3.12.4)

project(hazelcast_cpp_client
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
include(./cmake/utils.cmake)

# get and set Git commit date and id
get_commit_date_and_id(GIT_COMMIT_DATE GIT_COMMIT_ID)
message(STATUS "GIT_COMMIT_DATE = ${GIT_COMMIT_DATE}")
message(STATUS "GIT_COMMIT_ID = ${GIT_COMMIT_ID}")

# find all source and header files
FILE(GLOB_RECURSE SOURCE_FILES "./hazelcast/src/*cpp" "./hazelcast/generated-sources/src/*cpp")
FILE(GLOB_RECURSE HEADER_FILES "./hazelcast/include/*h" "./hazelcast/generated-sources/src/*h")

# options
option(WITH_OPENSSL
       "Use OpenSSL, setting this option to OFF causes SSL-related features to be disabled."
       ON)

option(BUILD_STATIC_LIB
       "Build static library."
       OFF)

option(BUILD_SHARED_LIB
       "Build shared library."
       ON)

option(DISABLE_LOGGING
       "Disable logging."
       OFF)

option(ENABLE_CLANG_TIDY
       "Enable clang-tidy linter."
       OFF)

option(BUILD_TESTS
       "Build tests."
       OFF)

option(BUILD_EXAMPLES
       "Build examples."
       OFF)

# exit with an error message if none of BUILD_SHARED_LIB and BUILD_STATIC_LIB was set.
if ((NOT BUILD_SHARED_LIB) AND (NOT BUILD_STATIC_LIB))
    message(FATAL_ERROR
            "Set at least one of BUILD_SHARED_LIB and BUILD_STATIC_LIB to ON.")
endif ()

# find dependencies

# find Boost
find_package(Boost REQUIRED COMPONENTS thread chrono)

# find OpenSSL if building WITH_OPENSSL
if (WITH_OPENSSL)
    if (APPLE)
        # This is a bug in CMake that causes it to prefer the system version over
        # the one in the specified ROOT folder. see
        # https://stackoverflow.com/questions/58446253/xcode-11-ld-error-your-binary-is-not-an-allowed-client-of-usr-lib-libcrypto-dy
        set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
        set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include CACHE FILEPATH "" FORCE)
        set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib CACHE FILEPATH "" FORCE)
        set(OPENSSL_SSL_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libssl.dylib CACHE FILEPATH "" FORCE)
    endif ()

    find_package(OpenSSL REQUIRED)
endif ()


function(add_hazelcast_library name type)
    # add the library target
    add_library(
        ${name}
        ${type}
        ${SOURCE_FILES} ${HEADER_FILES}
    )

    # library requires c++11
    target_compile_features(
        ${name}
        PUBLIC cxx_std_11
    )

    # MSVC-specific compiler flags
    if (MSVC)
        target_compile_options(${name} PRIVATE /bigobj)
    endif ()

    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        target_link_libraries(${name} PUBLIC pthread rt) # TODO make sure these are actually required
    endif ()

    # add Boost::thread and Boost::chrono as dependencies
    target_link_libraries(
        ${name}
        PUBLIC Boost::headers Boost::thread Boost::chrono
    )
    # set the Boost thread version
    target_compile_definitions(
        ${name}
        PUBLIC BOOST_THREAD_VERSION=5
    )

    # If building WITH_OPENSSL, add OpenSSL::SSL and OpenSSL::Crypto as dependencies
    # Both we and the user defines HZ_BUILD_WITH_SSL.
    if (WITH_OPENSSL)
        target_compile_definitions(
            ${name}
            PUBLIC HZ_BUILD_WITH_SSL
        )
        target_link_libraries(${name} PUBLIC OpenSSL::SSL OpenSSL::Crypto)
    endif()

    # windows-specific compile flags
    if (WIN32)
        # speeds the build process
        target_compile_definitions(${name} PRIVATE WIN32_LEAN_AND_MEAN)

        if (type STREQUAL "SHARED")
            # For the shared library,
            #   we define HAZELCAST_EXPORTS=1,
            #   users defines HAZELCAST_USE_SHARED
            target_compile_definitions(${name} PRIVATE HAZELCAST_EXPORTS=1)
            target_compile_definitions(${name} INTERFACE HAZELCAST_USE_SHARED)
        else () # type STREQUAL "STATIC"
            # For the static library,
            #   we define HAZELCAST_EXPORTS=0,
            #   users defines HAZELCAST_USE_STATIC
            target_compile_definitions(${name} PRIVATE HAZELCAST_EXPORTS=0)
            target_compile_definitions(${name} INTERFACE HAZELCAST_USE_STATIC)
        endif ()
    endif ()

    # add include directories
    target_include_directories(
        ${name}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/hazelcast/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/hazelcast/generated-sources/src>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    # add compile flags for version and git commit information
    target_compile_definitions(
        ${name}
        PRIVATE
            HAZELCAST_VERSION="${PROJECT_VERSION}"
            HAZELCAST_GIT_COMMIT_DATE=${GIT_COMMIT_DATE}
            HAZELCAST_GIT_COMMIT_ID=${GIT_COMMIT_ID}
    )

    if (DISABLE_LOGGING)
        target_compile_definitions(${name} PUBLIC HZ_LOGGING_DISABLED)
    endif ()

    # install library target
    install(
        TARGETS ${name}
        EXPORT ${name}-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # install the -targets.cmake file
    install(
        EXPORT ${name}-targets
        FILE ${name}-targets.cmake
        NAMESPACE hazelcast::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${name}
    )

    # configure -config-version.cmake file
    write_basic_package_version_file(
        ${name}-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    # configure -config.cmake file
    set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})
    set(LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
    configure_package_config_file(
        cmake/config.cmake.in
        ${name}-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${name}
        PATH_VARS INCLUDE_INSTALL_DIR LIBRARY_INSTALL_DIR
    )

    # install -config.cmake and -config-version.cmake files
    install(
        FILES
            ${CMAKE_BINARY_DIR}/${name}-config.cmake
            ${CMAKE_BINARY_DIR}/${name}-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${name}
    )
endfunction()

# the base name for shared and static libraries
set(BASE_LIBRARY_NAME hazelcastcxx)
# add the _ssl suffix to the base name if building WITH_OPENSSL
if (WITH_OPENSSL)
    set(BASE_LIBRARY_NAME ${BASE_LIBRARY_NAME}_ssl)
endif()

# set the names for the shared and static libraries
set(SHARED_LIBRARY_NAME ${BASE_LIBRARY_NAME})
set(STATIC_LIBRARY_NAME ${BASE_LIBRARY_NAME}_static)

# add shared library if requested
if (BUILD_SHARED_LIB)
    add_hazelcast_library(${SHARED_LIBRARY_NAME} SHARED)
endif()

# add static library if requested
if (BUILD_STATIC_LIB)
    add_hazelcast_library(${STATIC_LIBRARY_NAME} STATIC)
endif()

# install header files, this applies both to the shared and the static library
install(
    DIRECTORY
        hazelcast/include/
        hazelcast/generated-sources/src/
    TYPE INCLUDE
    FILES_MATCHING PATTERN "*.h"
)

# since shared and static libraries can both be requested for the build,
# we need a default one to use for tests and examples.
# the static library is preferred to the shared library, because it is OFF by default.
if (BUILD_STATIC_LIB)
    set(DEFAULT_LIBRARY_FOR_TESTS_AND_EXAMPLES ${STATIC_LIBRARY_NAME})
else ()
    set(DEFAULT_LIBRARY_FOR_TESTS_AND_EXAMPLES ${SHARED_LIBRARY_NAME})
endif ()

if (BUILD_TESTS)
    set(LIBRARY_FOR_TESTS ${DEFAULT_LIBRARY_FOR_TESTS_AND_EXAMPLES})

    option(INSTALL_GMOCK "Install Googletest's GMock?" OFF)
    option(INSTALL_GTEST "Install Googletest's GTest?" OFF)
    option(BUILD_GTEST "Install Googletest's GMock?" ON)
    option(BUILD_GMOCK "Install Googletest's GTest?" OFF)

    add_subdirectory(hazelcast/test)
endif ()

if (BUILD_EXAMPLES)
    set(LIBRARY_FOR_EXAMPLES ${DEFAULT_LIBRARY_FOR_TESTS_AND_EXAMPLES})

    add_subdirectory(examples)
endif ()
