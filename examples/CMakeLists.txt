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

cmake_minimum_required (VERSION 3.10)

project (hazelcast_cpp_client_examples
         VERSION 4.0.0
         DESCRIPTION "Hazelcast C++ Client Code Examples"
         HOMEPAGE_URL https://hazelcast.org/clients/cplusplus/
         LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT LIBRARY_FOR_EXAMPLES)
    message(FATAL_ERROR "Specify LIBRARY_FOR_EXAMPLES")
endif()

if (NOT TARGET ${LIBRARY_FOR_EXAMPLES})
    message(STATUS "${LIBRARY_FOR_EXAMPLES} is not a valid target, using find_package to find it.")
    find_package(${LIBRARY_FOR_EXAMPLES} REQUIRED)
    # the found target will have the namespace hazelcast::
    set(LIBRARY_FOR_EXAMPLES hazelcast::${LIBRARY_FOR_EXAMPLES})
endif()

if (MSVC)
    add_compile_options(/bigobj)
endif()

message(STATUS "Examples will be built with ${LIBRARY_FOR_EXAMPLES}")
link_libraries(${LIBRARY_FOR_EXAMPLES}) # TODO find a better way to do this

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
add_subdirectory(Org.Website.Samples)
add_subdirectory(replicated-map)
add_subdirectory(invocation-timeouts)
add_subdirectory(backpressure)
add_subdirectory(event-properties)
add_subdirectory(client-statistics)
add_subdirectory(pipeline)
add_subdirectory(authentication)
add_subdirectory(cp)
add_subdirectory(soak-test)

if (${LIBRARY_FOR_EXAMPLES} MATCHES "_ssl")
    add_subdirectory(tls)
    add_subdirectory(aws)
endif ()
