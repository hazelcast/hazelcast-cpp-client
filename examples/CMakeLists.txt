#
# Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
cmake_minimum_required (VERSION 3.10)
project (HazelcastExamples
        VERSION 4.0
        DESCRIPTION "Hazelcast C++ Client Code Examples"
        HOMEPAGE_URL https://hazelcast.org/clients/cplusplus/
        LANGUAGES CXX)

INCLUDE(TestBigEndian)
INCLUDE(FindOpenSSL)

# if HAZELCAST_INSTALL_DIR option is provided, the examples shall compile with the released library
if (NOT("${HAZELCAST_INSTALL_DIR}x" STREQUAL "x"))
    message (STATUS "Using hazelcast installation at ${HAZELCAST_INSTALL_DIR}.")

    if("${HZ_VERSION}x" STREQUAL "x")
        message(FATAL_ERROR "Release verification requires that the HZ_VERSION variable is provided and this folder is NOT included from a parent CMakeLists.txt.")
    endif ()

    if ("${HZ_BIT}x" STREQUAL "x")
        message(FATAL_ERROR "You have to specify HZ_BIT variable!")
    endif()

    IF(NOT ("${HZ_LIB_TYPE}" STREQUAL "STATIC" OR "${HZ_LIB_TYPE}" STREQUAL "SHARED") )
        message( STATUS "Build needs HZ_LIB_TYPE. Setting default as -DHZ_LIB_TYPE=STATIC (other option -DHZ_LIB_TYPE=SHARED)" )
        set(HZ_LIB_TYPE STATIC)
    ENDIF()

    IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        SET(HAZELCAST_PLATFORM_INSTALL_DIR ${HAZELCAST_INSTALL_DIR}\\${CMAKE_SYSTEM_NAME}_${HZ_BIT})
    ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        SET(HAZELCAST_PLATFORM_INSTALL_DIR ${HAZELCAST_INSTALL_DIR}/Mac_${HZ_BIT})
    ELSE()
        SET(HAZELCAST_PLATFORM_INSTALL_DIR ${HAZELCAST_INSTALL_DIR}/${CMAKE_SYSTEM_NAME}_${HZ_BIT})
    ENDIF()

    SET(HAZELCAST_INCLUDE_DIRECTORIES ${HAZELCAST_PLATFORM_INSTALL_DIR}/hazelcast/include ${HAZELCAST_PLATFORM_INSTALL_DIR}/external/include/easyloggingpp/src)
    if (${HZ_COMPILE_WITH_SSL} MATCHES "ON")
        SET(HAZELCAST_LIB_DIRECTORY ${HAZELCAST_PLATFORM_INSTALL_DIR}/hazelcast/lib/tls)
    else()
        SET(HAZELCAST_LIB_DIRECTORY ${HAZELCAST_PLATFORM_INSTALL_DIR}/hazelcast/lib)
    endif()

    message(STATUS "Shall try building the examples with installed Hazelcast client at ${HAZELCAST_PLATFORM_INSTALL_DIR}")
    message(STATUS "Include directories: ${HAZELCAST_INCLUDE_DIRECTORIES}")
    message(STATUS "Link directory: ${HAZELCAST_LIB_DIRECTORY}")

    #detect endianness
    TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
    if (${IS_BIG_ENDIAN})
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DHZ_BIG_ENDIAN")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DHZ_BIG_ENDIAN")
    ENDIF (${IS_BIG_ENDIAN})

    include_directories(SYSTEM ${HAZELCAST_INCLUDE_DIRECTORIES})

    SET(HZ_LIB_NAME HazelcastClient${HZ_VERSION}_${HZ_BIT})

    ADD_LIBRARY(${HZ_LIB_NAME} ${HZ_LIB_TYPE} IMPORTED)

    message(STATUS "${CMAKE_SYSTEM}")

    if (HZ_COMPILE_WITH_SSL)
        message(STATUS "Building the with SSL support.")
        ADD_DEFINITIONS(-DHZ_BUILD_WITH_SSL)
        find_package(OpenSSL 1.0.2 REQUIRED)
        include_directories(${OPENSSL_INCLUDE_DIR})
        link_libraries(${OPENSSL_SSL_LIBRARIES} ${OPENSSL_CRYPTO_LIBRARIES})
    ELSE ()
        message(STATUS "Building without SSL.")
    ENDIF()

    IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        message(STATUS "APPLE ENVIRONMENT DETECTED")

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")

        set(HZ_LIB_PATH "")
        IF (${HZ_LIB_TYPE} MATCHES "STATIC")
            set(HZ_LIB_PATH "${HAZELCAST_LIB_DIRECTORY}/lib${HZ_LIB_NAME}.a")
        ELSE(${HZ_LIB_TYPE} MATCHES "STATIC")
            set(HZ_LIB_PATH "${HAZELCAST_LIB_DIRECTORY}/lib${HZ_LIB_NAME}.dylib")
        ENDIF(${HZ_LIB_TYPE} MATCHES "STATIC")

        message(STATUS "Using ${HZ_LIB_TYPE} library at ${HZ_LIB_PATH}")

        set_property(TARGET ${HZ_LIB_NAME} PROPERTY IMPORTED_LOCATION "${HZ_LIB_PATH}")

        link_libraries(${HZ_LIB_NAME})
    ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    IF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
        message(STATUS "LINUX ENVIRONMENT DETECTED")

        SET(HZ_BIT_FLAG " ")
        IF(${HZ_BIT} MATCHES "32")
            SET(HZ_BIT_FLAG " -m32 ")
        ENDIF(${HZ_BIT} MATCHES "32")

        message(STATUS "${HZ_BIT} Bit")

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS ${HZ_BIT_FLAG}")

        set(HZ_LIB_PATH "")
        IF (${HZ_LIB_TYPE} MATCHES "STATIC")
            set(HZ_LIB_PATH "${HAZELCAST_LIB_DIRECTORY}/lib${HZ_LIB_NAME}.a")
        ELSE(${HZ_LIB_TYPE} MATCHES "STATIC")
            set(HZ_LIB_PATH "${HAZELCAST_LIB_DIRECTORY}/lib${HZ_LIB_NAME}.so")
        ENDIF(${HZ_LIB_TYPE} MATCHES "STATIC")

        message(STATUS "Using ${HZ_LIB_TYPE} library at ${HZ_LIB_PATH}")

        set_property(TARGET ${HZ_LIB_NAME} PROPERTY IMPORTED_LOCATION "${HZ_LIB_PATH}")

        link_libraries(${HZ_LIB_NAME} pthread rt )
    ENDIF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

    IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        message(STATUS "WINDOWS ENVIRONMENT DETECTED ${CMAKE_GENERATOR} ${CMAKE_BUILD_TYPE} ")

        add_definitions(-D__WIN${HZ_BIT}__ -DWIN${HZ_BIT} -D_WIN${HZ_BIT})

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /EHsc")

        add_definitions(-DHAZELCAST_EXPORTS=0)

        IF (${HZ_LIB_TYPE} MATCHES "STATIC")
            set(HZ_LIB_PATH "${HAZELCAST_LIB_DIRECTORY}\\static\\${HZ_LIB_NAME}.lib")
            set_property(TARGET ${HZ_LIB_NAME} PROPERTY IMPORTED_LOCATION ${HZ_LIB_PATH})

            message(STATUS "Using ${HZ_LIB_TYPE} library at ${HZ_LIB_PATH}")
        ELSE(${HZ_LIB_TYPE} MATCHES "STATIC")
            set(HZ_LIB_PATH "${HAZELCAST_LIB_DIRECTORY}\\shared\\${HZ_LIB_NAME}.dll")
            set(HZ_LIB_IMPLIB_PATH "${HAZELCAST_LIB_DIRECTORY}\\shared\\${HZ_LIB_NAME}.lib")

            set_property(TARGET ${HZ_LIB_NAME} PROPERTY IMPORTED_LOCATION ${HZ_LIB_PATH})
            set_property(TARGET ${HZ_LIB_NAME} PROPERTY IMPORTED_IMPLIB ${HZ_LIB_IMPLIB_PATH})

            message(STATUS "Using ${HZ_LIB_TYPE} library at ${HZ_LIB_PATH}")
        ENDIF(${HZ_LIB_TYPE} MATCHES "STATIC")

        link_libraries(${HZ_LIB_NAME})
    ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
endif (NOT("${HAZELCAST_INSTALL_DIR}x" STREQUAL "x"))

IF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed -ldl")
ENDIF()

add_subdirectory(command-line-tool)
add_subdirectory(transactions)
add_subdirectory(spi)
add_subdirectory(serialization)
add_subdirectory(network-configuration)
add_subdirectory(monitoring)
add_subdirectory(learning-basics)
add_subdirectory(distributed-topic)
add_subdirectory(distributed-primitives)
add_subdirectory(distributed-map)
add_subdirectory(distributed-collections)
add_subdirectory(adaptor)
add_subdirectory(Org.Website.Samples)
add_subdirectory(replicated-map)
add_subdirectory(invocation-timeouts)
add_subdirectory(backpressure)
add_subdirectory(event-properties)
add_subdirectory(client-statistics)
add_subdirectory(pipeline)
if (${HZ_COMPILE_WITH_SSL} MATCHES "ON")
    add_subdirectory(tls)
    add_subdirectory(aws)
endif()







