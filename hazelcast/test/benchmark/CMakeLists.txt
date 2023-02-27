#
# Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

project(hazelcast-cpp-client-benchmark
        VERSION 4.1.1
        DESCRIPTION "Hazelcast C++ Client Benchmark Project"
        LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(benchmark REQUIRED)
find_package(hazelcast-cpp-client REQUIRED)

add_executable(hazelcast_benchmark hazelcast_benchmark.cpp)
target_link_libraries(hazelcast_benchmark benchmark::benchmark benchmark::benchmark_main hazelcast-cpp-client::hazelcast-cpp-client)
