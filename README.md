<p align="center">
    <a href="https://github.com/hazelcast/hazelcast-cpp-client/">
        <img src="https://docs.hazelcast.com/_/img/hazelcast-header.svg" />
    </a>
    <h2 align="center">Hazelcast C++ Client</h2>
</p>

<p align="center">
    <a href="https://hazelcastcommunity.slack.com/channels/cpp-client">
        <img src="https://img.shields.io/badge/slack-chat-green.svg" alt="Chat on Slack">
    </a>
    <a href="https://twitter.com/Hazelcast">
        <!-- markdownlint-disable-next-line MD013 -->
        <img src="https://img.shields.io/twitter/follow/Hazelcast.svg?style=flat-square&colorA=1da1f2&colorB=&label=Follow%20on%20Twitter" alt="Follow on Twitter">
    </a>
</p>

---

## What is Hazelcast?

[Hazelcast](https://hazelcast.com/) is a distributed computation and storage platform for consistently low-latency querying,
aggregation and stateful computation against event streams and traditional data sources. It allows you to quickly build
resource-efficient, real-time applications. You can deploy it at any scale from small edge devices to a large cluster of
cloud instances.

A cluster of Hazelcast nodes share both the data storage and computational load which can dynamically scale up and down.
When you add new nodes to the cluster, the data is automatically rebalanced across the cluster, and currently running
computational tasks (known as jobs) snapshot their state and scale with processing guarantees.

For more info, check out Hazelcast [repository](https://github.com/hazelcast/hazelcast).


# Hazelcast C++ Client

Hazelcast is an open-source distributed in-memory data store and computation platform. It provides a wide variety of distributed data structures and concurrency primitives.

hazelcast-cpp-client is the official C++ library API for using the Hazelcast in-memory database platform. It requires C++11 support.  

The library can be installed using package managers [Conan](https://github.com/hazelcast/hazelcast-cpp-client/blob/master/Reference_Manual.md#111-conan-users) and [Vcpkg](https://github.com/hazelcast/hazelcast-cpp-client/blob/master/Reference_Manual.md#112-vcpkg-users) or directly [from source code](https://github.com/hazelcast/hazelcast-cpp-client/blob/master/Reference_Manual.md#113-install-from-source-code-using-cmake) using [CMake](https://cmake.org/).

* [Hazelcast Community Slack - C++ Client Channel](https://hazelcastcommunity.slack.com/channels/cpp-client)
## Installation
### Hazelcast
Hazelcast C++ client requires a working Hazelcast cluster to run. This cluster handles the storage and
manipulation of the user data.

A Hazelcast cluster consists of one or more cluster members. These members generally run on multiple virtual or
physical machines and are connected to each other via the network. Any data put on the cluster is partitioned to
multiple members transparent to the user. It is therefore very easy to scale the system by adding new members as
the data grows. Hazelcast cluster also offers resilience. Should any hardware or software problem causes a crash
to any member, the data on that member is recovered from backups and the cluster continues to operate without any
downtime.

The quickest way to start a single member cluster for development purposes is to use our
[Docker images](https://hub.docker.com/r/hazelcast/hazelcast/).

```bash
docker run -p 5701:5701 hazelcast/hazelcast
```

This command fetches the latest Hazelcast version. You can find all available tags
[here](https://hub.docker.com/r/hazelcast/hazelcast/tags).

You can also use our ZIP or TAR [distributions](https://hazelcast.com/open-source-projects/downloads/)
as described [here](DOCUMENTATION.md#121-setting-up-a-hazelcast-cluster).

### Client

#### Installing

##### Vcpkg Users
Hazelcast C++ client package is available for [Vcpkg](https://github.com/microsoft/vcpkg) users. The port name is `hazelcast-cpp-client`.

Please see [Getting Started](https://github.com/microsoft/vcpkg#getting-started) on how to use Vcpkg package manager with your application. In summary,

If you use Linux or Mac:

```sh
git clone https://github.com/microsoft/vcpkg
./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install "hazelcast-cpp-client[openssl]" --recurse
``` 

If you use Windows:

```bat
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install "hazelcast-cpp-client[openssl]:x64-windows" --recurse
``` 
The above code snippet will install `hazelcast-cpp-client` with its `boost` dependencies.

After the installation, the library is available for usage. For example, if you are using CMake for your builds, you can use the following cmake build command with the `CMAKE_TOOLCHAIN_FILE` cmake option to be the `vcpkg.cmake`.
```bat
> cmake -B [build directory] -S . -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
> cmake --build [build directory]
```

The above code will install `hazelcast-cpp-client` with its `boost` and `openssl` dependencies.

You can find more details on using a Vcpkg installed package from different IDEs in your projects from the [Vcpkg Official Getting Started](https://github.com/microsoft/vcpkg#getting-started) documentation.

##### Other Methods

You can also install the hazelcast-cpp-client with Conan and from source code. You can more information from [Reference Manual](https://github.com/akeles85/hazelcast-cpp-client/blob/readme_update/Reference_Manual.md#11-installing).

## Overview

### Usage

There is an example project in sample_project directory. You can run the example as below:

If you use Linux or Mac:

```sh
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake &&
cmake --build build &&
./build/client
```

If you use Windows:

```bat
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]\scripts\buildsystems\vcpkg.cmake && 
cmake --build build && 
.\build\Debug\client
```

The sample code creates a client, the client automatically connects to the cluster.
It creates a map named "personel_map" and puts the records inside it.
It then gets all the entries from the cluster and prints them.
```c++
#include <hazelcast/client/hazelcast_client.h>
int main() {
    auto hz = hazelcast::new_client().get(); // Connects to the cluster

    auto personel = hz.get_map("personel_map").get();
    personel->put<std::string, std::string>("Alice", "IT").get();
    personel->put<std::string, std::string>("Bob", "IT").get();
    personel->put<std::string, std::string>("Clark", "IT").get();
    std::cout << "Added IT personel. Logging all known personel" << std::endl;
    for (const auto &entry : personel->entry_set<std::string, std::string>().get()) {
        std::cout << entry.first << " is in " << entry.second << " department." << std::endl;
    }
    
    return 0;
}
```

## Features

* Distributed, partitioned and queryable in-memory key-value store implementation, called **Map**
* Eventually consistent cache implementation to store a subset of the Map data locally in the memory of the client, called **Near Cache**
* Additional data structures and simple messaging constructs such as **Set**, **MultiMap**, **Queue**, **Topic**
* Cluster-wide unique ID generator, called **FlakeIdGenerator**
* Distributed, CRDT based counter, called **PNCounter**
* Distributed concurrency primitives from CP Subsystem such as **FencedLock**, **Semaphore**, **AtomicLong**
* Integration with [Hazelcast Cloud](https://cloud.hazelcast.com/)
* Support for serverless and traditional web service architectures with **Unisocket** and **Smart** operation modes
* Ability to listen client lifecycle, cluster state and distributed data structure events
* and [many more](https://hazelcast.com/clients/cplusplus/#client-features).

## Documentation

You can find the detailed documentation at the [documentation site](https://hazelcast.github.io/hazelcast-cpp-client/doc-index.html) and the [API reference](https://hazelcast.github.io/hazelcast-cpp-client/api-index.html).

## License

hazelcast-cpp-client library is an open source project using the [Apache 2 License](https://github.com/hazelcast/hazelcast-cpp-client/blob/master/LICENSE).

## Copyright

Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.

Visit [www.hazelcast.com](http://www.hazelcast.com) for more information.
