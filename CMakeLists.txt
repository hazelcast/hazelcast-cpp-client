#
# Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

cmake_minimum_required(VERSION 3.10)

project(hazelcast-cpp-client
        VERSION 4.0.1
        DESCRIPTION "Hazelcast C++ Client"
        LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the default build type for single-config generators if not given
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if ((NOT is_multi_config) AND (NOT CMAKE_BUILD_TYPE))
    message(STATUS "CMAKE_BUILD_TYPE was not set, using Release as the default.")
    set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
include(GenerateExportHeader)
include(./cmake/utils.cmake)

# get and set Git commit date and id
get_commit_date_and_id(GIT_COMMIT_DATE GIT_COMMIT_ID)
message(STATUS "GIT_COMMIT_DATE = ${GIT_COMMIT_DATE}")
message(STATUS "GIT_COMMIT_ID = ${GIT_COMMIT_ID}")

# find all source and header files
FILE(GLOB_RECURSE SOURCE_FILES
     "./hazelcast/src/*cpp"
     "./hazelcast/generated-sources/src/*cpp")
FILE(GLOB_RECURSE HEADER_FILES
     "./hazelcast/include/*h"
     "./hazelcast/generated-sources/src/*h")

# options
option(WITH_OPENSSL
       "Build with OpenSSL. Setting this option to ON enables SSL-related features."
       OFF)

option(BUILD_SHARED_LIBS
       "Build shared library."
       ON)

option(DISABLE_LOGGING
       "Disable logging."
       OFF)

option(BUILD_TESTS
       "Build tests."
       OFF)

option(BUILD_EXAMPLES
       "Build examples."
       OFF)

# find dependencies

# find Threads
find_package(Threads REQUIRED)

# find Boost
find_package(Boost 1.71 REQUIRED COMPONENTS thread chrono)

# find OpenSSL if building WITH_OPENSSL
if (WITH_OPENSSL)
    if (APPLE)
        # This is a bug in CMake that causes it to prefer the system version over
        # the one in the specified ROOT folder.
        # See https://stackoverflow.com/a/62063357
        set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
    endif ()

    find_package(OpenSSL REQUIRED)
endif ()

# add library
if (BUILD_SHARED_LIBS)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

# add the library target
add_library(
        ${PROJECT_NAME}
        ${LIB_TYPE}
        ${SOURCE_FILES} ${HEADER_FILES}
)

# set library's version and soversion
set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION}
)

# library requires c++11
target_compile_features(
        ${PROJECT_NAME}
        PUBLIC cxx_std_11
)

# links the library against the system's thread library
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)

# add Boost::thread and Boost::chrono as dependencies
target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC Boost::boost Boost::thread Boost::chrono
)
# set the Boost thread version
target_compile_definitions(
        ${PROJECT_NAME}
        PUBLIC BOOST_THREAD_VERSION=5
)

# If building WITH_OPENSSL, add OpenSSL::SSL and OpenSSL::Crypto as dependencies
# Both we and the user defines HZ_BUILD_WITH_SSL.
if (WITH_OPENSSL)
    target_compile_definitions(
            ${PROJECT_NAME}
            PUBLIC HZ_BUILD_WITH_SSL
    )
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenSSL::SSL OpenSSL::Crypto)
endif()

# MSVC-specific compiler flags
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /bigobj)
endif ()

# windows-specific compile flags
if (WIN32)
    # speeds the build process
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN)
endif ()

# add include directories
target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/hazelcast/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/hazelcast/generated-sources/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# add compile flags for version and git commit information
target_compile_definitions(
        ${PROJECT_NAME}
        PRIVATE
        HAZELCAST_VERSION="${PROJECT_VERSION}"
        HAZELCAST_GIT_COMMIT_DATE=${GIT_COMMIT_DATE}
        HAZELCAST_GIT_COMMIT_ID=${GIT_COMMIT_ID}
)

if (DISABLE_LOGGING)
    target_compile_definitions(${PROJECT_NAME} PUBLIC HZ_LOGGING_DISABLED)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES DEFINE_SYMBOL HAZELCAST_EXPORTS)

generate_export_header(
        ${PROJECT_NAME}
        BASE_NAME hazelcast
        EXPORT_MACRO_NAME HAZELCAST_API
        EXPORT_FILE_NAME include/hazelcast/util/export.h
        NO_EXPORT_MACRO_NAME HAZELCAST_PRIVATE
        STATIC_DEFINE HAZELCAST_USE_STATIC
)

if (type STREQUAL "STATIC")
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAZELCAST_USE_STATIC)
endif()

# install library target
install(
        TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# install the -targets.cmake file
install(
        EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# configure -config-version.cmake file
write_basic_package_version_file(
        ${PROJECT_NAME}-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

# configure -config.cmake file
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set(LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
configure_package_config_file(
        cmake/config.cmake.in
        ${PROJECT_NAME}-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        PATH_VARS INCLUDE_INSTALL_DIR LIBRARY_INSTALL_DIR
)

# install -config.cmake and -config-version.cmake files
install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# install header files, this applies both to the shared and the static library
install(
    DIRECTORY
        hazelcast/include/
        hazelcast/generated-sources/src/
        ${CMAKE_CURRENT_BINARY_DIR}/include/

    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

if (BUILD_TESTS)
    add_subdirectory(hazelcast/test)
endif ()

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

message(STATUS "Configuration summary:")
if (NOT is_multi_config)
    message(STATUS "  CMAKE_BUILD_TYPE =      ${CMAKE_BUILD_TYPE}")
endif()
message(STATUS "  BUILD_SHARED_LIBS =     ${BUILD_SHARED_LIBS}")
message(STATUS "  WITH_OPENSSL =          ${WITH_OPENSSL}")
message(STATUS "  DISABLE_LOGGING =       ${DISABLE_LOGGING}")
message(STATUS "  CMAKE_INSTALL_PREFIX =  ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  BUILD_TESTS =           ${BUILD_TESTS}")
message(STATUS "  BUILD_EXAMPLES =        ${BUILD_EXAMPLES}")
