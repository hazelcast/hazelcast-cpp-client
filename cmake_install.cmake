# Install script for directory: /Users/ihsan/Desktop/work/src/hazelcast-cpp-client/examples

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Library/Developer/CommandLineTools/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/transactions/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/spi/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/serialization/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/network-configuration/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/monitoring/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/learning-basics/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/distributed-topic/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/distributed-primitives/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/distributed-map/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/distributed-collections/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/Org.Website.Samples/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/replicated-map/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/invocation-timeouts/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/backpressure/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/event-properties/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/client-statistics/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/pipeline/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/authentication/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/cp/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/soak-test/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/external-smart-client-discovery/cmake_install.cmake")
  include("/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/sql/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/ihsan/Desktop/work/src/hazelcast-cpp-client/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
