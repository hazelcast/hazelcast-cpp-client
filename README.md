<p align="center">
    <a href="https://hazelcast.com">
        <img class="center" src="https://docs.hazelcast.com/_/img/hazelcast-logo.svg" alt="logo">
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
    <a href="LICENSE">
        <img src="https://img.shields.io/badge/License-Apache_2.0-blue.svg" alt="Chat on Slack">
    </a>    
</p>

---

# What is Hazelcast?

[Hazelcast](https://hazelcast.com/) is a distributed computation and storage platform for consistently low-latency querying,
aggregation and stateful computation against event streams and traditional data sources. It allows you to quickly build
resource-efficient, real-time applications. You can deploy it at any scale from small edge devices to a large cluster of
cloud instances.

A cluster of Hazelcast nodes share both the data storage and computational load which can dynamically scale up and down.
When you add new nodes to the cluster, the data is automatically rebalanced across the cluster, and currently running
computational tasks (known as jobs) snapshot their state and scale with processing guarantees.

For more information, see the Hazelcast [repository](https://github.com/hazelcast/hazelcast).


# Hazelcast C++ Client

For an introduction to the C++ client for Hazelcast, and information on how to install and get started with the client, see the [Hazelcast documentation](https://docs.hazelcast.com/hazelcast/latest/clients/cplusplus).

hazelcast-cpp-client is the official C++ library API for using the Hazelcast in-memory database platform. It requires C++11 support.  


## Features

* Distributed, partitioned and queryable in-memory key-value store implementation, called [Map](examples/distributed-map/basic/FillMap.cpp)
* Eventually consistent cache implementation to store a subset of the Map data locally in the memory of the client, called [Near Cache](examples/distributed-map/near-cache)
* Additional data structures and simple messaging constructs such as [Set](examples/distributed-collections/set), [MultiMap](examples/distributed-map/multimap/MultimapPut.cpp), [Queue](examples/distributed-collections/blockingqueue), [Topic](examples/distributed-topic)
* Cluster-wide unique ID generator, called [FlakeIdGenerator](examples/learning-basics/unique-names)
* Distributed, CRDT based counter, called [PNCounter](examples/distributed-primitives/crdt-pncounter)
* Distributed concurrency primitives from CP Subsystem such as [FencedLock](examples/cp/fenced_lock.cpp), [Semaphore](examples/cp/counting_semphore.cpp), [AtomicLong](examples/cp/atomic_long.cpp)
* Integration with [Viridian](https://viridian.hazelcast.com/) (Hazelcast Cloud)
* Support for serverless and traditional web service architectures with **Unisocket** and **Smart** operation modes
* Ability to listen to client lifecycle, cluster state and distributed data structure events
* and [many more](https://hazelcast.com/clients/cplusplus/#client-features).

## Documentation

For information about:

* how to install and get started with the client, see the [Hazelcast documentation](https://docs.hazelcast.com/hazelcast/latest/clients/cplusplus)
* how to configure and use the C++ client, see the Reference Manual in this repo
* the API, see the [API reference](https://hazelcast.github.io/hazelcast-cpp-client/api-index.html)


## Copyright

Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.

Visit [www.hazelcast.com](http://www.hazelcast.com) for more information.
