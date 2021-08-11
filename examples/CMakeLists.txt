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

cmake_minimum_required (VERSION 3.10)

project (hazelcast-cpp-client-examples
         VERSION 4.1.1
         DESCRIPTION "Hazelcast C++ Client Code Examples"
         LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT TARGET hazelcast-cpp-client)
    message(STATUS "hazelcast-cpp-client is not a valid target, using find_package to find it.")
    find_package(hazelcast-cpp-client REQUIRED)
    link_libraries(hazelcast-cpp-client::hazelcast-cpp-client) # TODO find a better way to do this
else()
    link_libraries(hazelcast-cpp-client) # TODO find a better way to do this
endif()

if (MSVC)
    add_compile_options(/bigobj)
endif()

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
add_subdirectory(external-smart-client-discovery)

if (${WITH_OPENSSL})
    add_subdirectory(tls)
    add_subdirectory(aws)
    add_subdirectory(cloud-discovery)
endif ()
