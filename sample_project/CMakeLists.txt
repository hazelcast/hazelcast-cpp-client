cmake_minimum_required(VERSION 3.10)

project(HazelcastClientSample)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(hazelcast-cpp-client CONFIG REQUIRED)


add_executable(client client.cpp)
target_link_libraries(client PRIVATE hazelcast-cpp-client::hazelcast-cpp-client)