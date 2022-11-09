# Hazelcast C++ Client Library
<!--ts-->
   * [Hazelcast C++ Client Library](#hazelcast-c-client-library)
   * [Introduction](#introduction)
   * [Resources](#resources)
   * [Release Notes](#release-notes)
   * [1. Getting Started](#1-getting-started)
      * [1.1. Installing](#11-installing)
         * [1.1.1. Conan Users](#111-conan-users)
         * [1.1.2. Vcpkg Users](#112-vcpkg-users)
         * [1.1.3. Install From Source Code Using CMake](#113-install-from-source-code-using-cmake)
            * [1.1.3.1. Requirements](#1131-requirements)
            * [1.1.3.2. Downloading Source Code](#1132-downloading-source-code)
            * [1.1.3.3. Linux and MacOS Users](#1133-linux-and-macos-users)
            * [1.1.3.4. Windows Users](#1134-windows-users)
            * [1.1.3.5. Advanced Installation](#1135-advanced-installation)
               * [1.1.3.5.1. Custom Install Location](#11351-custom-install-location)
               * [1.1.3.5.2. CMake Configuration](#11352-cmake-configuration)
      * [1.2. Starting Hazelcast IMDG Cluster](#12-starting-hazelcast-imdg-cluster)
         * [1.2.1. Starting Hazelcast Server](#121-starting-hazelcast-server)
            * [1.2.1.1. Starting Server Using Hazelcast Docker Images](#1211-starting-server-using-hazelcast-docker-images)
            * [1.2.1.2. Starting Server Using Hazelcast Command Line Tools](#1212-starting-server-using-hazelcast-command-line-tools)
            * [1.2.1.3. Starting Server Using Hazelcast Distribution](#1213-starting-server-using-hazelcast-distribution)
            * [1.2.1.4. Adding User Java Library to Java CLASSPATH](#1214-adding-user-java-library-to-java-classpath)
         * [1.3. Compiling Your Project](#13-compiling-your-project)
            * [1.3.1. CMake Users](#131-cmake-users)
            * [1.3.2. Linux and MacOS Users](#132-linux-and-macos-users)
            * [1.3.3. Windows Users](#133-windows-users)
      * [1.4. Basic Configuration](#14-basic-configuration)
         * [1.4.1. Configuring Hazelcast Server](#141-configuring-hazelcast-server)
         * [1.4.2. Configuring Hazelcast C++ Client](#142-configuring-hazelcast-c-client)
         * [1.4.2.1 Cluster Name](#1421-cluster-name)
         * [1.4.2.2. Network Settings](#1422-network-settings)
         * [1.4.3. Client System Properties](#143-client-system-properties)
      * [1.5. Basic Usage](#15-basic-usage)
      * [1.6. Code Samples](#16-code-samples)
   * [2. Features](#2-features)
   * [3. Configuration Overview](#3-configuration-overview)
      * [3.1. Configuration Options](#31-configuration-options)
         * [3.1.1. Programmatic Configuration](#311-programmatic-configuration)
   * [4. Serialization](#4-serialization)
      * [4.1. identified_data_serializer Serialization](#41-identified_data_serializer-serialization)
      * [4.2. portable_serializer Serialization](#42-portable_serializer-serialization)
      * [4.3. Custom Serialization](#43-custom-serialization)
      * [4.4. JSON Serialization](#44-json-serialization)
      * [4.5. Global Serialization](#45-global-serialization)
   * [5. Setting Up Client Network](#5-setting-up-client-network)
      * [5.1. Providing Member Addresses](#51-providing-member-addresses)
      * [5.2. Setting Smart Routing](#52-setting-smart-routing)
      * [5.3. Enabling Redo Operation](#53-enabling-redo-operation)
      * [5.4. Setting Cluster Connection Timeout](#54-setting-cluster-connection-timeout)
      * [5.5. Advanced Cluster Connection Retry Configuration](#55-advanced-cluster-connection-retry-configuration)
      * [5.6. Enabling Client TLS/SSL](#56-enabling-client-tlsssl)
      * [5.7. Enabling Hazelcast AWS Discovery](#57-enabling-hazelcast-aws-discovery)
      * [5.8. Enabling Hazelcast Cloud Discovery](#58-enabling-hazelcast-cloud-discovery)
      * [5.8.1. Cloud Discovery With SSL Enabled](#581-cloud-discovery-with-ssl-enabled)
      * [5.9. External Smart Client Discovery](#59-external-smart-client-discovery)
      * [5.10. Authentication](#510-authentication)
      * [5.10.1. Username Password Authentication](#5101-username-password-authentication)
      * [5.10.2. Token Authentication](#5102-token-authentication)
      * [5.11. Configuring Backup Acknowledgment](#511-configuring-backup-acknowledgment)
   * [6. Securing Client Connection](#6-securing-client-connection)
      * [6.1. TLS/SSL](#61-tlsssl)
         * [6.1.1. TLS/SSL for Hazelcast Members](#611-tlsssl-for-hazelcast-members)
         * [6.1.2. TLS/SSL for Hazelcast C++ Client](#612-tlsssl-for-hazelcast-c-client)
            * [6.1.2.1. Enabling Validation Of Server Certificate](#6121-enabling-validation-of-server-certificate)
            * [6.1.2.2. Mutual Authentication (Two Way Authentication)](#6122-mutual-authentication-two-way-authentication)
            * [6.1.2.3. Constraining The Used Cipher List](#6123-constraining-the-used-cipher-list)
   * [7. Using C++ Client with Hazelcast IMDG](#7-using-c-client-with-hazelcast-imdg)
      * [7.1. C++ Client API Overview](#71-c-client-api-overview)
      * [7.2. C++ Client Operation Modes](#72-c-client-operation-modes)
         * [7.2.1. Smart Client](#721-smart-client)
         * [7.2.2. Unisocket Client](#722-unisocket-client)
      * [7.3. Handling Failures](#73-handling-failures)
         * [7.3.1. Handling Client Connection Failure](#731-handling-client-connection-failure)
         * [7.3.2. Handling Retry-able Operation Failure](#732-handling-retry-able-operation-failure)
         * [7.3.3. Client Backpressure](#733-client-backpressure)
      * [7.3.4 Client Connection Strategy](#734-client-connection-strategy)
         * [7.3.4.1 Configuring Client Reconnect Strategy](#7341-configuring-client-reconnect-strategy)
      * [7.4. Using Distributed Data Structures](#74-using-distributed-data-structures)
         * [7.4.1. Using imap](#741-using-imap)
         * [7.4.2. Using multi_map](#742-using-multi_map)
         * [7.4.3. Using replicated_map](#743-using-replicated_map)
         * [7.4.4. Using iqueue](#744-using-iqueue)
         * [7.4.5. Using iset](#745-using-iset)
         * [7.4.6. Using ilist](#746-using-ilist)
         * [7.4.7. Using ringbuffer](#747-using-ringbuffer)
         * [7.4.8. Using reliable_topic](#748-using-reliable_topic)
         * [7.4.9 Using pn_counter](#749-using-pn_counter)
         * [7.4.10 Using flake_id_generator](#7410-using-flake_id_generator)
         * [7.4.11. CP Subsystem](#7411-cp-subsystem)
            * [7.4.11.1. Using atomic_long](#74111-using-atomic_long)
            * [7.4.11.2. Using fenced_lock](#74112-using-fenced_lock)
            * [7.4.11.3. Using counting_semaphore](#74113-using-counting_semaphore)
            * [7.4.11.4. Using latch](#74114-using-latch)
            * [7.4.11.5. Using atomic_reference](#74115-using-atomic_reference)
         * [7.4.12. Using Transactions](#7412-using-transactions)
      * [7.5. Distributed Events](#75-distributed-events)
         * [7.5.1. Cluster Events](#751-cluster-events)
            * [7.5.1.1. Listening for Member Events](#7511-listening-for-member-events)
            * [7.5.1.3. Listening for Lifecycle Events](#7513-listening-for-lifecycle-events)
         * [7.5.2. Distributed Data Structure Events](#752-distributed-data-structure-events)
            * [7.5.2.1. Listening for Map Events](#7521-listening-for-map-events)
      * [7.6. Distributed Computing](#76-distributed-computing)
         * [7.6.1. Distributed Executor Service](#761-distributed-executor-service)
            * [7.6.1.1 Implementing a Callable Task](#7611-implementing-a-callable-task)
            * [7.6.1.2 Executing a Callable Task](#7612-executing-a-callable-task)
            * [7.6.1.3 Scaling The Executor Service](#7613-scaling-the-executor-service)
            * [7.6.1.4 Executing Code in the Cluster](#7614-executing-code-in-the-cluster)
            * [7.6.1.5 Canceling an Executing Task](#7615-canceling-an-executing-task)
            * [7.6.1.5.1 Example Task to Cancel](#76151-example-task-to-cancel)
            * [7.6.1.6 Selecting Members for Task Execution](#7616-selecting-members-for-task-execution)
         * [7.6.2. Using entry_processor](#762-using-entry_processor)
            * [7.6.2.1. Processing Entries](#7621-processing-entries)
      * [7.7. Distributed Query](#77-distributed-query)
         * [7.7.1. How Distributed Query Works](#771-how-distributed-query-works)
            * [7.7.1.1. person Map Query Example](#7711-person-map-query-example)
            * [7.7.1.2. Querying by Combining Predicates with AND, OR, NOT](#7712-querying-by-combining-predicates-with-and-or-not)
            * [7.7.1.3. Querying with SQL](#7713-querying-with-sql)
               * [7.7.1.3.1. Supported SQL Syntax](#77131-supported-sql-syntax)
            * [7.7.1.4. Querying Examples with Predicates](#7714-querying-examples-with-predicates)
            * [7.7.1.5. Querying with JSON Strings](#7715-querying-with-json-strings)
            * [7.7.1.6. Filtering with Paging Predicates](#7716-filtering-with-paging-predicates)
      * [7.8. Performance](#78-performance)
         * [7.8.1. Partition Aware](#781-partition-aware)
         * [7.8.2. Near Cache](#782-near-cache)
            * [7.8.2.1 Configuring Near Cache](#7821-configuring-near-cache)
            * [7.8.2.2. Near Cache Example for Map](#7822-near-cache-example-for-map)
            * [7.8.2.3. Near Cache Eviction](#7823-near-cache-eviction)
            * [7.8.2.4. Near Cache Expiration](#7824-near-cache-expiration)
            * [7.8.2.5. Near Cache Invalidation](#7825-near-cache-invalidation)
         * [7.8.3. Pipelining](#783-pipelining)
      * [7.9. Monitoring and Logging](#79-monitoring-and-logging)
         * [7.9.1. Enabling Client Statistics](#791-enabling-client-statistics)
         * [7.9.2. Logging Configuration](#792-logging-configuration)
      * [7.10. Mixed Object Types Supporting Hazelcast Client](#710-mixed-object-types-supporting-hazelcast-client)
         * [7.10.1. typed_data API](#7101-typed_data-api)
      * [7.11. Sql Module](#711-sql-module)
         * [7.11.1 Overview](#7111-overview)
         * [7.11.2 Create Mapping](#7112-create-mapping)
         * [7.11.3 Read SELECT Results](#7113-read-select-results)
         * [7.11.4 Sql Statement With Options](#7114-sql-statement-with-options)
         * [7.11.5 Read Row Metadata](#7115-read-row-metadata)
   * [8. Development and Testing](#8-development-and-testing)
      * [8.1. Testing](#81-testing)
   * [9. Getting Help](#9-getting-help)
   * [10. Contributing](#10-contributing)
   * [11. License](#11-license)
   * [12. Copyright](#12-copyright)

<!-- Added by: ihsan, at: Mon Aug 23 15:55:05 +03 2021 -->

<!--te-->

# Introduction

This document provides information about the C++ client for [Hazelcast](https://hazelcast.com/). This client uses Hazelcast's [Open Client Protocol](https://github.com/hazelcast/hazelcast-client-protocol) and works with Hazelcast IMDG 4.0 and higher versions.

The client API is fully asynchronous. The API returns `boost::future` API which has the capability of [continuations](https://www.boost.org/doc/libs/1_74_0/doc/html/thread/synchronization.html#thread.synchronization.futures.then). If the user wants to make sure that the requested operation is completed in the cluster and committed in the distributed database, he/she needs to wait for the result of the future.

# Resources

See the following for more information on Hazelcast IMDG:

* Hazelcast IMDG [website](https://hazelcast.com)
* Hazelcast IMDG [Reference Manual](https://docs.hazelcast.com/imdg/latest)

# Release Notes

See the [Releases](https://github.com/hazelcast/hazelcast-cpp-client/releases) page of this repository.

# 1. Getting Started

This chapter provides information on how to get started with your Hazelcast C++ client. It outlines the requirements, installation and configuration of the client, setting up a cluster, and provides a simple application that uses a distributed map in C++ client.

## 1.1. Installing

### 1.1.1. Conan Users
Hazelcast C++ client package is indexed at [Conan Center Index](https://conan.io/center/hazelcast-cpp-client). You can use [Conan package manager](https://conan.io/) to install Hazelcast C++ client. The package name is `hazelcast-cpp-client`.

Please see [example instructions](https://docs.conan.io/en/latest/getting_started.html#an-md5-hash-calculator-using-the-poco-libraries) on how to use conan package manager with your application. In summary,

- You need to put the following lines to your `conanfile.txt`:
```
[requires]
hazelcast-cpp-client/5.0.0

[generators]
cmake
```
- Then, you execute the following
```
$ mkdir build && cd build
$ conan install ..
```
This generates the `conanbuildinfo.cmake` file to be included in your CMakeLists.txt. Please follow the instructions at the [example page](https://docs.conan.io/en/latest/getting_started.html#an-md5-hash-calculator-using-the-poco-libraries) and build your application.

### 1.1.2. Vcpkg Users
Hazelcast C++ client package is available for [Vcpkg](https://github.com/microsoft/vcpkg) users. The port name is `hazelcast-cpp-client`.

Please see [Getting Started](https://github.com/microsoft/vcpkg#getting-started) on how to use Vcpkg package manager with your application. In summary,

```bat
> git clone https://github.com/microsoft/vcpkg
> .\vcpkg\bootstrap-vcpkg.bat
> .\vcpkg\vcpkg install hazelcast-cpp-client
``` 
The above code snippet will install `hazelcast-cpp-client` with its `boost` dependencies.

After the installation, the library is available for usage. For example, if you are using CMake for your builds, you can use the following cmake build command with the `CMAKE_TOOLCHAIN_FILE` cmake option to be the `vcpkg.cmake`.
```bat
> cmake -B [build directory] -S . -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
> cmake --build [build directory]
```

You can find more details on using a Vcpkg installed package from different IDEs in your projects from the [Vcpkg Official Getting Started](https://github.com/microsoft/vcpkg#getting-started) documentation.

If you need to use `openssl` feature, then you need to install using the following command:
```bat
> .\vcpkg\vcpkg install hazelcast-cpp-client[openssl]
```
The above code will install `hazelcast-cpp-client` with its `boost` and `openssl` dependencies.

### 1.1.3. Install From Source Code Using CMake
#### 1.1.3.1. Requirements
1. Linux, macOS or Windows
2. A compiler that supports C++11
3. [CMake](https://cmake.org) 3.10 or above
4. [Boost](https://www.boost.org) 1.71 or above
5. [OpenSSL](https://www.openssl.org) (optional)

#### 1.1.3.2. Downloading Source Code
Go to the [releases](https://github.com/hazelcast/hazelcast-cpp-client/releases) page to download the source code for the latest Hazelcast C++ client.

The releases page has both `tar.gz` and `zip` archives available. Choose the one which suits your system the best.

Follow the instructions for your platform:
* [Linux and MacOS](#1133-linux-and-macos-users)
* [Windows](#1134-windows-users)

#### 1.1.3.3. Linux and MacOS Users
Here is how you download and extract version 5.0.0 using the **curl** command:
```sh
curl -Lo hazelcast-cpp-client-5.0.0.tar.gz https://github.com/hazelcast/hazelcast-cpp-client/archive/v5.0.0.tar.gz
tar xzf hazelcast-cpp-client-5.0.0.tar.gz
```

Alternatively, you may clone the repository and checkout a specific version:
```sh
git clone https://github.com/hazelcast/hazelcast-cpp-client.git
cd hazelcast-cpp-client
git checkout v5.0.0
```

Once you are in the source directory of the Hazelcast C++ client library, create and change into a new directory:
```sh
cd hazelcast-cpp-client-5.0.0
mkdir build
cd build
```

Run `cmake` (or `cmake3` if you are on CentOS or RHEL) to configure:
```sh
cmake ..
```
See the [advanced installation](#1135-advanced-installation) section for configuration options.

Run `cmake` again to build and install the library:
```sh
cmake --build .
sudo cmake --build . --target install
```
See [this section](#11351-custom-install-location) for information on how to use a different installation location.

#### 1.1.3.4. Windows Users
Download and extract the release archive from the 
[releases](https://github.com/hazelcast/hazelcast-cpp-client/releases) page.

Open a `cmd` window and change into the folder where you extracted the contents of the release archive. Then create and change into a new directory:
```bat
cd hazelcast-cpp-client-5.0.0
mkdir build
cd build
```

Run `cmake` to configure:
```bat
cmake ..
``` 
See the [advanced installation](#1135-advanced-installation) section for configuration options.

Build and install:
```bat
cmake --build . --config Release
cmake --build . --target install --config Release
```
The above commands will build and install the library with the `Release` configuration. Make sure you pass the same `--config` option to both commands.

The install command may require administrator privileges depending on your install prefix. See [this section](#11351-custom-install-location) for information on how to use a different installation location.

#### 1.1.3.5. Advanced Installation

##### 1.1.3.5.1. Custom Install Location
Pass the argument `-DCMAKE_INSTALL_PREFIX=/path/to/install` the first time you run `cmake` to configure the installation directory:
```sh
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install
```

##### 1.1.3.5.2. CMake Configuration
You can provide additional configuration options using the `-DVARIABLE=VALUE` syntax on the command line. Here are all the options that are supported:
* `WITH_OPENSSL` : Set to `ON` to build the library with SSL support.
This will require [OpenSSL](https://www.openssl.org) to be installed on your system. The default is `OFF`.
* `BUILD_SHARED_LIBS` : Set to `ON` or `OFF` depending on whether you want the shared(ON) or static(OFF) library. The default is `ON`.
* `DISABLE_LOGGING` : Setting this option to `ON` disables logging. The default is `OFF`.

For example, if you want to build the static library with SSL support, you can use the following command:

```sh
cmake .. -DWITH_OPENSSL=ON -DBUILD_SHARED_LIBS=OFF
```

## 1.2. Starting Hazelcast IMDG Cluster

Hazelcast C++ client requires a working Hazelcast IMDG cluster to run. This cluster handles storage and manipulation of the user data. Clients are a way to connect to the Hazelcast IMDG cluster and access such data.

Hazelcast IMDG cluster consists of one or more cluster members. These members generally run on multiple virtual or physical machines and are connected to each other via network. Any data put on the cluster is partitioned to multiple members transparent to the user. It is therefore very easy to scale the system by adding new members as the data grows. Hazelcast IMDG cluster also offers resilience. Should any hardware or software problem causes a crash to any member, the data on that member is recovered from backups and the cluster continues to operate without any downtime. Hazelcast clients are an easy way to connect to a Hazelcast IMDG cluster and perform tasks on distributed data structures that live on the cluster.

In order to use Hazelcast C++ client, we first need to setup a Hazelcast IMDG servers.

### 1.2.1. Starting Hazelcast Server

#### 1.2.1.1. Starting Server Using Hazelcast Docker Images

The quickest way to start a single member cluster for development purposes is to use our [Docker images](https://hub.docker.com/r/hazelcast/hazelcast/).

```bash
docker run -p 5701:5701 hazelcast/hazelcast:4.2
```

#### 1.2.1.2. Starting Server Using Hazelcast Command Line Tools

Hazelcast Command Line is a tool which allows users to install & run Hazelcast IMDG and Management Center on local environment. Please follow the [Command Line Tool Instructions](https://github.com/hazelcast/hazelcast-command-line) to start the Hazelcast cluster.  

#### 1.2.1.3. Starting Server Using Hazelcast Distribution

Follow the instructions below to create a Hazelcast IMDG cluster:

1. Go to Hazelcast's download [page](https://hazelcast.com/open-source-projects/downloads/) and download either the `.zip` or `.tar` distribution of Hazelcast IMDG.
2. Decompress the contents into any directory that you want to run members from.
3. Change into the directory that you decompressed the Hazelcast content and then into the `bin` directory.
4. Use either `start.sh` or `start.bat` depending on your operating system. Once you run the start script, you should see the Hazelcast IMDG logs in the terminal.

You should see a log similar to the following, which means that your 1-member cluster is ready to be used:

```
Nov 19, 2020 2:52:59 PM com.hazelcast.internal.cluster.ClusterService
INFO: [192.168.1.112]:5701 [dev] [5.0.0] 

Members {size:1, ver:1} [
        Member [192.168.1.112]:5701 - 360ba49b-ef33-4590-9abd-ceff3e31dc06 this
]

Nov 19, 2020 2:52:59 PM com.hazelcast.core.LifecycleService
INFO: [192.168.1.112]:5701 [dev] [5.0.0] [192.168.1.112]:5701 is STARTED
```

#### 1.2.1.4. Adding User Java Library to Java CLASSPATH

When you want to use features such as querying and language interoperability, you might need to add your own Java classes to the Hazelcast member in order to use them from your C++ client. This can be done by adding your own compiled code to the `CLASSPATH`. To do this, compile your code with the `CLASSPATH` and add the compiled files to the `user-lib` directory in the extracted `hazelcast-<version>.zip` (or `tar`). Then, you can start your Hazelcast member by using the start scripts in the `bin` directory. The start scripts will automatically add your compiled classes to the `CLASSPATH`.

Note that if you are adding an `IdentifiedDataSerializable` or a `Portable` class, you need to add its factory too. Then, you should configure the factory in the `hazelcast.xml` configuration file. This file resides in the `bin` directory where you extracted the `hazelcast-<version>.zip` (or `tar`).

The following is an example configuration when you are adding an `IdentifiedDataSerializable` class:

```xml
<hazelcast>
     ...
     <serialization>
        <data-serializable-factories>
            <data-serializable-factory factory-id="66">
                com.hazelcast.client.test.IdentifiedFactory
            </data-serializable-factory>
        </data-serializable-factories>
    </serialization>
    ...
</hazelcast>
```
If you want to add a `Portable` class, you should use `<portable-factories>` instead of `<data-serializable-factories>` in the above configuration.

See the [Hazelcast IMDG Reference Manual](https://docs.hazelcast.com/imdg/latest/getting-started.html) for more information on setting up the clusters.

### 1.3. Compiling Your Project

If you are using CMake, see the section for [CMake users](#131-cmake-users).

If you are not, then read the instructions specific to your platform:
* [Linux and MacOS](#132-linux-and-macos-users)
* [Windows](#133-windows-users)

#### 1.3.1. CMake Users
A Hazelcast IMDG C++ client installation comes with package configuration files for CMake. If your project is using CMake, you can easily find and link against the client library:
```cmake
find_package(hazelcast-cpp-client CONFIG REQUIRED)

target_link_libraries(mytarget PRIVATE hazelcast-cpp-client::hazelcast-cpp-client)
```

Make sure you add the installation prefix of the client library to `CMAKE_PREFIX_PATH` if you are using a custom installation location. 

#### 1.3.2. Linux and MacOS Users
You can pass the `-lhazelcast-cpp-client` option to the compiler to link against the client library. 

The client library depends on Boost.Thread and Boost.Chrono. You should also link your program against these libraries using `-lboost_thread` and `-lboost_chrono`. The Boost.Thread library should be provided with the preprocessor definition `BOOST_THREAD_VERSION=5` for necessary features such as futures and future continuations to be enabled. 

Here is how you can compile an example from the examples directory:
```sh
g++ -std=c++11 \
    examples/path/to/example.cpp \
    -DBOOST_THREAD_VERSION=5 \
    -lhazelcast-cpp-client -lboost_thread -lboost_chrono
``` 

If a custom installation directory was used during installation, then you may also need to use the `-L` and `-I` options to add the library and include paths to the compiler's search path.
```
g++ -std=c++11 \
    examples/path/to/example.cpp \
    -I /path/to/install/include -L /path/to/install/lib \
    -lhazelcast-cpp-client -lboost_thread -lboost_chrono 
```

#### 1.3.3. Windows Users
Provide your compiler with the include directories and library files for the Hazelcast IMDG C++ client and its dependencies.

You also need to pass the preprocessor definition `BOOST_THREAD_VERSION=5` for necessary features such as futures and future continuations to be enabled. 

The following is a command that can be used to compile an example from the examples directory.
```bat
cl.exe path\to\example.cpp ^
    C:\path\to\hazelcast\lib\hazelcast-cpp-client.lib ^
    C:\path\to\boost\lib\boost_thread.lib C:\path\to\boost\lib\boost_chrono.lib ^
    /EHsc /DBOOST_THREAD_VERSION=5 ^
    /I C:\path\to\hazelcast\include /I C:\path\to\boost\include
```

## 1.4. Basic Configuration

If you are using Hazelcast IMDG and C++ Client on the same computer, generally the default configuration should be fine. This is great for trying out the client. However, if you run the client on a different computer than any of the cluster members, you may need to do some simple configurations such as specifying the member addresses.

The Hazelcast IMDG members and clients have their own configuration options. You may need to reflect some of the member side configurations on the client side to properly connect to the cluster.

This section describes the most common configuration elements to get you started in no time. It discusses some member side configuration options to ease the understanding of Hazelcast's ecosystem. Then, the client side configuration options regarding the cluster connection are discussed. The configurations for the Hazelcast IMDG data structures that can be used in the C++ client are discussed in the following sections.

See the [Hazelcast IMDG Reference Manual](https://docs.hazelcast.com/imdg/latest) and [Configuration Overview section](#3-configuration-overview) for more information.

### 1.4.1. Configuring Hazelcast Server

Hazelcast IMDG aims to run out-of-the-box for most common scenarios. However, if you have limitations on your network such as multicast being disabled, you may have to configure your Hazelcast IMDG members so that they can find each other on the network. Also, since most of the distributed data structures are configurable, you may want to configure them according to your needs. We will show you the basics about network configuration here.

You can use the following options to configure Hazelcast IMDG:

* Using the `hazelcast.xml` configuration file.
* Programmatically configuring the member before starting it from the Java code.

Since we use standalone servers, we will use the `hazelcast.xml` file to configure our cluster members.

When you download and unzip `hazelcast-<version>.zip` (or `tar`), you see the `hazelcast.xml` in the `bin` directory. When a Hazelcast member starts, it looks for the `hazelcast.xml` file to load the configuration from. A sample `hazelcast.xml` is shown below.

```xml
<hazelcast>
    <cluster-name>dev</cluster-name>

    <network>
        <port auto-increment="true" port-count="100">5701</port>
        <join>
            <multicast enabled="true">
                <multicast-group>224.2.2.3</multicast-group>
                <multicast-port>54327</multicast-port>
            </multicast>
            <tcp-ip enabled="false">
                <interface>127.0.0.1</interface>
                <member-list>
                    <member>127.0.0.1</member>
                </member-list>
            </tcp-ip>
        </join>
        <ssl enabled="false"/>
    </network>
    <partition-group enabled="false"/>
    <map name="default">
        <backup-count>1</backup-count>
    </map>
</hazelcast>
```

We will go over some important configuration elements in the rest of this section.

- `<cluster-name>`: Specifies which cluster this member belongs to. 

- `<network>`
    - `<port>`: Specifies the port number to be used by the member when it starts. Its default value is 5701. You can specify another port number, and if you set `auto-increment` to `true`, then Hazelcast will try the subsequent ports until it finds an available port or the `port-count` is reached.
    - `<join>`: Specifies the strategies to be used by the member to find other cluster members. Choose which strategy you want to
    use by setting its `enabled` attribute to `true` and the others to `false`.
        - `<multicast>`: Members find each other by sending multicast requests to the specified address and port. It is very useful if IP addresses of the members are not static.
        - `<tcp>`: This strategy uses a pre-configured list of known members to find an already existing cluster. It is enough for a member to find only one cluster member to connect to the cluster. The rest of the member list is automatically retrieved from that member. We recommend putting multiple known member addresses there to avoid disconnectivity should one of the members in the list is unavailable at the time of connection.

These configuration elements are enough for most connection scenarios. Now we will move onto the configuration of the C++ client.

### 1.4.2. Configuring Hazelcast C++ Client

You must configure a Hazelcast C++ Client programmatically. Config files of any type are not yet supported for the C++ client.

You can start the client with no custom configuration like this:

```c++
    auto hz = hazelcast::new_client().get(); // Connects to the cluster
```

This section describes some network configuration settings to cover common use cases in connecting the client to a cluster. See the [Configuration Overview section](#3-configuration-overview) and the following sections for information about detailed network configurations and/or additional features of Hazelcast C++ client configuration.

An easy way to configure your Hazelcast C++ Client is to create a `client_config` object and set the appropriate options. Then you need to pass this object to the client when starting it, as shown below.

```c++
    hazelcast::client::client_config config;
    config.set_cluster_name("my-cluster"); // the server is configured to use the `my_cluster` as the cluster name hence we need to match it to be able to connect to the server.
    config.get_network_config().add_address(address("192.168.1.10", 5701));
    auto hz = hazelcast::new_client(std::move(config)).get(); // Connects to the cluster member at ip address `192.168.1.10` and port 5701
```

If you run the Hazelcast IMDG members in a different server than the client, you most probably have configured the members' ports and cluster names as explained in the previous section. If you did, then you need to make certain changes to the network settings of your client.

### 1.4.2.1 Cluster Name

You only need to provide the name of the cluster if it is explicitly configured on the server side (otherwise the default value of `dev` is used).

```c++
hazelcast::client::client_config config;
config.set_cluster_name("my-cluster"); // the server is configured to use the `my_cluster` as the cluster name hence we need to match it to be able to connect to the server.
```

### 1.4.2.2. Network Settings

You need to provide the IP address and port of at least one member in your cluster so the client can find it.

```c++
hazelcast::client::client_config config;
config.get_network_config().add_address(hazelcast::client::address("your server ip", 5701 /* your server port*/));
```
### 1.4.3. Client System Properties

While configuring your C++ client, you can use various system properties provided by Hazelcast to tune its clients. These properties can be set programmatically through `config.set_property` or by using an environment variable. The value of this property will be:

* the programmatically configured value, if programmatically set,
* the environment variable value, if the environment variable is set,
* the default value, if none of the above is set.

See the following for an example client system property configuration:

**Programmatically:**

```c++
config.set_property(hazelcast::client::client_properties::INVOCATION_TIMEOUT_SECONDS, "2") // Sets invocation timeout as 2 seconds
```

or 

```c++
config.set_property("hazelcast.client.invocation.timeout.seconds", "2") // Sets invocation timeout as 2 seconds
```

**By using an environment variable on Linux:** 

```sh
export hazelcast.client.invocation.timeout.seconds=2
```

If you set a property both programmatically and via an environment variable, the programmatically set value will be used.

See the [complete list of system properties](https://github.com/hazelcast/hazelcast-cpp-client/blob/master/hazelcast/include/hazelcast/client/client_properties.h), along with their descriptions, which can be used to configure your Hazelcast C++ client.

## 1.5. Basic Usage

Now that we have a working cluster and we know how to configure both our cluster and client, we can run a simple program to use a distributed map in C++ client.

The following example first creates a programmatic configuration object. Then, it starts a client.

```c++
#include <hazelcast/client/hazelcast_client.h>
int main() {
    auto hz = hazelcast::new_client().get(); // Connects to the cluster
    std::cout << "Started the Hazelcast C++ client instance " << hz.get_name() << std::endl; // Prints client instance name
    return 0;
}
```
This should print logs about the cluster members and information about the client itself such as client type and local address port.
```
18/11/2020 21:22:26.835 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/spi.cpp:375] (Wed Nov 18 17:25:23 2020 +0300:3b11bea) LifecycleService::LifecycleEvent Client (75121987-12fe-4ede-860d-59222e6d3ef2) is STARTING
18/11/2020 21:22:26.835 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/spi.cpp:379] (Wed Nov 18 17:25:23 2020 +0300:3b11bea) LifecycleService::LifecycleEvent STARTING
18/11/2020 21:22:26.835 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/spi.cpp:387] LifecycleService::LifecycleEvent STARTED
18/11/2020 21:22:26.837 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/network.cpp:587] Trying to connect to Address[10.212.1.117:5701]
18/11/2020 21:22:26.840 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/spi.cpp:411] LifecycleService::LifecycleEvent CLIENT_CONNECTED
18/11/2020 21:22:26.840 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/network.cpp:637] Authenticated with server  Address[:5701]:a27f900e-b1eb-48be-aa46-d7a4922ef704, server version: 4.2, local address: Address[10.212.1.116:37946]
18/11/2020 21:22:26.841 INFO: [139868341360384] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/spi.cpp:881]

Members [1]  {
        Member[10.212.1.117]:5701 - a27f900e-b1eb-48be-aa46-d7a4922ef704
}

Started the Hazelcast C++ client instance hz.client_1
```
Congratulations! You just started a Hazelcast C++ Client.

**Using a Map**

Let's manipulate a distributed map on a cluster using the client.

Save the following file as `IT.cpp` and compile it using a command similar to the following (Linux g++ compilation is used for demonstration):

```c++
g++ IT.cpp -o IT -lhazelcast-cpp-client -lboost_thread -lboost_chrono -DBOOST_THREAD_VERSION=5
```
Then, you can run the application using the following command:
 
```
./IT
```

**main.cpp**

```c++
#include <hazelcast/client/hazelcast_client.h>
int main() {
    auto hz = hazelcast::new_client().get(); // Connects to the cluster

    auto personnel = hz.get_map("personnel_map").get();
    personnel->put<std::string, std::string>("Alice", "IT").get();
    personnel->put<std::string, std::string>("Bob", "IT").get();
    personnel->put<std::string, std::string>("Clark", "IT").get();
    std::cout << "Added IT personnel. Logging all known personnel" << std::endl;
    for (const auto &entry : personnel->entry_set<std::string, std::string>().get()) {
        std::cout << entry.first << " is in " << entry.second << " department." << std::endl;
    }
    
    return 0;
}
```

**Output**

```
18/11/2020 21:22:26.835 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/spi.cpp:375] (Wed Nov 18 17:25:23 2020 +0300:3b11bea) LifecycleService::LifecycleEvent Client (75121987-12fe-4ede-860d-59222e6d3ef2) is STARTING
18/11/2020 21:22:26.835 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/spi.cpp:379] (Wed Nov 18 17:25:23 2020 +0300:3b11bea) LifecycleService::LifecycleEvent STARTING
18/11/2020 21:22:26.835 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/spi.cpp:387] LifecycleService::LifecycleEvent STARTED
18/11/2020 21:22:26.837 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/network.cpp:587] Trying to connect to Address[10.212.1.117:5701]
18/11/2020 21:22:26.840 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/spi.cpp:411] LifecycleService::LifecycleEvent CLIENT_CONNECTED
18/11/2020 21:22:26.840 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/network.cpp:637] Authenticated with server  Address[:5701]:a27f900e-b1eb-48be-aa46-d7a4922ef704, server version: 4.2, local address: Address[10.212.1.116:37946]
18/11/2020 21:22:26.841 INFO: [139868341360384] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/spi.cpp:881]

Members [1]  {
        Member[10.212.1.117]:5701 - a27f900e-b1eb-48be-aa46-d7a4922ef704
}
Added IT personnel. Logging all known personnel
Alice is in IT department
Clark is in IT department
Bob is in IT department
```

You see this example puts all the IT personnel into a cluster-wide `personnel_map` and then prints all the known personnel.

Now create a `Sales.cpp` file, compile and run it as shown below.

**Compile:**

```c++
g++ Sales.cpp -o Sales -lhazelcast-cpp-client -lboost_thread -lboost_chrono -DBOOST_THREAD_VERSION=5
```
**Run**

Then, you can run the application using the following command:

```
./Sales
```

**Sales.cpp**

```c++
#include <hazelcast/client/hazelcast_client.h>
int main() {
auto hz = hazelcast::new_client().get(); // Connects to the cluster

auto personnel = hz.get_map("personnel_map").get();
    personnel->put<std::string, std::string>("Denise", "Sales").get();
    personnel->put<std::string, std::string>("Erwing", "Sales").get();
    personnel->put<std::string, std::string>("Fatih", "Sales").get();
    personnel->put<std::string, std::string>("Bob", "IT").get();
    personnel->put<std::string, std::string>("Clark", "IT").get();
    std::cout << "Added all sales personnel. Logging all known personnel" << std::endl;
    for (const auto &entry : personnel.entry_set().get()) {
        std::cout << entry.first << " is in " << entry.second << " department." << std::endl;
    }

    return 0;
}
```

**Output**

```
18/11/2020 21:22:26.835 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/spi.cpp:375] (Wed Nov 18 17:25:23 2020 +0300:3b11bea) LifecycleService::LifecycleEvent Client (75121987-12fe-4ede-860d-59222e6d3ef2) is STARTING
18/11/2020 21:22:26.835 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/spi.cpp:379] (Wed Nov 18 17:25:23 2020 +0300:3b11bea) LifecycleService::LifecycleEvent STARTING
18/11/2020 21:22:26.835 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/spi.cpp:387] LifecycleService::LifecycleEvent STARTED
18/11/2020 21:22:26.837 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/network.cpp:587] Trying to connect to Address[10.212.1.117:5701]
18/11/2020 21:22:26.840 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/spi.cpp:411] LifecycleService::LifecycleEvent CLIENT_CONNECTED
18/11/2020 21:22:26.840 INFO: [139868602337152] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/network.cpp:637] Authenticated with server  Address[:5701]:a27f900e-b1eb-48be-aa46-d7a4922ef704, server version: 4.2, local address: Address[10.212.1.116:37946]
18/11/2020 21:22:26.841 INFO: [139868341360384] client_1[dev] [5.0.0] [/home/ihsan/hazelcast-cpp-client/hazelcast/src/hazelcast/client/spi.cpp:881]

Members [1]  {
        Member[10.212.1.117]:5701 - a27f900e-b1eb-48be-aa46-d7a4922ef704
}
Added Sales personnel. Logging all known personnel
Denise is in Sales department
Erwing is in Sales department
Fatih is in Sales department
Bob is in IT department
Clark is in IT department
```

You will see this time we add only the sales employees but we get the list all known employees including the ones in IT. That is because our map lives in the cluster and no matter which client we use, we can access the whole map.

## 1.6. Code Samples

See the Hazelcast C++ [code samples](https://github.com/hazelcast/hazelcast-cpp-client/tree/master/examples) for more examples.

# 2. Features

Hazelcast C++ client supports the following data structures and features:

* imap
* iqueue
* iset
* ilist
* multi_map
* replicated_map
* Ringbuffer
* reliable_topic
* CRDT pn_counter
* flake_id_generator
* fenced_lock (CP Subsystem)
* counting_semaphore (CP Subsystem)
* atomic_long (CP Subsystem)
* atomic_reference (CP Subsystem)
* latch (CP Subsystem)
* Event Listeners
* Distributed Executor Service
* Entry Processor
* transactional_map
* transactional_multi_map
* transactional_queue
* transactional_list
* transactional_set
* Query (Predicates)
* paging_predicate
* Built-in Predicates
* Listener with predicate
* Near Cache Support
* Programmatic Configuration
* Fail Fast on Invalid Configuration
* SSL Support (requires Enterprise server)
* Authorization
* Smart Client
* Unisocket Client
* Lifecycle Service
* identified_data_serializer Serialization
* portable_serializer Serialization
* custom_serializer Serialization
* JSON Serialization
* Global Serialization
* Hazelcast AWS Discovery
* Hazelcast Cloud Discovery
* External Smart Client Discovery

# 3. Configuration Overview

This chapter describes the options to configure your C++ client.

## 3.1. Configuration Options

You can configure Hazelcast C++ client programmatically (API).

### 3.1.1. Programmatic Configuration

For programmatic configuration of the Hazelcast C++ client, just instantiate a `client_config` object and configure the desired aspects. An example is shown below.

```c++
hazelcast::client::client_config config;
config.get_network_config().add_address({ "your server ip", 5701 /* your server port*/});
auto hz = hazelcast::new_client(std::move(config)).get(); // Connects to the cluster
```

See the `client_config` class reference at the Hazelcast C++ client API Documentation for details.

# 4. Serialization

Serialization is the process of converting an object into a stream of bytes to store the object in the memory, a file or database, or transmit it through the network. Its main purpose is to save the state of an object in order to be able to recreate it when needed. The reverse process is called deserialization. Hazelcast offers you its own native serialization methods. You will see these methods throughout this chapter.

Hazelcast serializes all your objects before sending them to the server. The `unsigned char` (`byte`), `bool`, `char`, `short`, `int32_t`, `int64_t`, `float`, `double`, `std::string` types are serialized natively and you cannot override this behavior. The following table is the conversion of types for Java server side.

| C++          | Java                                |
|--------------|-------------------------------------|
| byte         | Byte                                |
| bool         | Boolean                             |
| char         | Character                           |
| short        | Short                               |
| int32_t      | Integer                             |
| int64_t      | Long                                |
| float        | Float                               |
| double       | Double                              |
| std::string  | String                              |

Vector of the above types can be serialized as `boolean[]`, `byte[]`, `short[]`, `int[]`, `float[]`, `double[]`, `long[]` and `string[]` for the Java server side, respectively. 

If you want the serialization to work faster or you use the clients in different languages, Hazelcast offers its own native serialization types, such as [`identified_data_serializer` serialization](#41-identified_data_serializer-serialization) and [`portable_serializer` serialization](#42-portable_serializer-serialization).

On top of all, if you want to use your own serialization type, you can use a [Custom Serialization](#43-custom-serialization).

Hazelcast serialization is mostly performed at compile time by implementing the specialization of template struct `hz_serializer<>`. The specialized struct should be defined at the `hazelcast::client::serialization` namespace. The serializer should be derived from one of the following marker classes:

1. identified_data_serializer
2. portable_serializer
3. custom_serializer
4. json serialization
5. global_serializer

This way of specialization allows us two advantages:

1. No need to modify your classes (no extra API interfaces for existing classes).

2. The determination of the serializer is at compile time.  

In addition to deriving from these marker classes, certain methods should be implemented in order your code to compile with this serializations. 

You can also configure a global serializer in case non-of the above serializations are implemented for the user object. If no serializer can be found, then `exception::hazelcast_serialization` is thrown at runtime.

## 4.1. identified_data_serializer Serialization

For a faster serialization of objects, Hazelcast recommends to implement the `identified_data_serializer` interface. The following is an example of a `hz_serializer` implementing this interface:

```c++
struct Person {
    std::string name;
    bool male;
    int32_t age;
};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<Person> : identified_data_serializer {
                static int32_t get_factory_id() noexcept {
                    return 1;
                }

                static int32_t get_class_id() noexcept {
                    return 3;
                }

                static void write_data(const Person &object, object_data_output &out) {
                    out.write(object.name);
                    out.write(object.male);
                    out.write(object.age);
                }

                static Person read_data(object_data_input &in) {
                    return Person{in.read<std::string>(), in.read<bool>(), in.read<int32_t>()};
                }
            };
        }
    }
}
```

`hz_serializer<Person>` specialization of `hz_serializer` should implement the above four methods, namely `get_factory_id`, `get_class_id`, `write_data`, `read_data`. In case that the object fields are non-public, you can always define struct `hz_serializer<Person>` as a friend to your object class. You use the `object_data_output` class methods when serializing the object into binary bytes and the `object_data_input` class methods when de-serializing the bytes into the concrete object instance. The factory and class ID returned from the associated methods should match the IDs defined at the server side (you need to have the corresponding Java classes at the server side).

## 4.2. portable_serializer Serialization

If you want to support versioning of the objects, then you can use the portable serialization type. It has the following benefits:

- Support multiversion of the same object type.
- Fetch individual fields without having to rely on the reflection.
- Querying and indexing support without deserialization and/or reflection.

In order to support these features, a serialized `Portable` object contains meta information like the version and concrete location of the each field in the binary data. This way Hazelcast is able to navigate in the binary data and deserialize only the required field without actually deserializing the whole object which improves the query performance.

With multiversion support, you can have two members where each of them having different versions of the same object, and Hazelcast will store both meta information and use the correct one to serialize and deserialize portable objects depending on the member. This is very helpful when you are doing a rolling upgrade without shutting down the cluster.

Also note that portable serialization is totally language independent and is used as the binary protocol between Hazelcast server and clients.

A sample portable_serializer implementation of a `Person` class looks like the following:

```c++
struct Person {
    std::string name;
    bool male;
    int32_t age;
};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<Person> : portable_serializer {
                static int32_t get_factory_id() noexcept {
                    return 1;
                }

                static int32_t get_class_id() noexcept {
                    return 3;
                }

                static void write_portable(const Person &object, portable_writer &out) {
                out.write("name", object.name);
                    out.write("gender", object.male);
                    out.write("age", object.age);
                }

                static Person read_portable(portable_reader &in) {
                    return Person{in.read<std::string>("name"), in.read<bool>("gender"), in.read<int32_t>("age")};
                }
            };
        }
    }
}

```

Note that, this is very similar to `identified_data_serializer` implementation except the method signatures and the derived marker class of `portable_serializer` for the specialized serializer.

`hz_serializer<Person>` specialization of `hz_serializer` should implement the above four methods, namely `get_factory_id`, `get_class_id`, `write_portable`, `read_portable`. In case that the object fields are non-public, you can always define struct `hz_serializer<Person>` as friend to your object class. You use the `portable_writer` class methods when serializing the object into binary bytes and you use the `portable_reader` class methods when de-serializing the bytes into the concrete object instance. 

## 4.3. Custom Serialization

Hazelcast lets you plug a custom serializer to be used for serialization of objects. It allows you alsoan integration point for any external serialization frameworks such as protobuf, flatbuffers, etc. 

A sample custom_serializer implementation of a `Person` class looks like the following:

```c++
struct Person {
    std::string name;
    bool male;
    int32_t age;
};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<Person> : custom_serializer {
                static constexpr int32_t get_type_id() noexcept {
                    return 3;
                }

                static void write(const Person &object, object_data_output &out) {
                    out.write(object.name);
                    out.write(object.male);
                    out.write(object.age);
                }

                static Person read(object_data_input &in) {
                    return Person{in.read<std::string>(), in.read<bool>(), in.read<int32_t>()};
                }
            };
        }
    }
}
```

`hz_serializer<Person>` specialization of `hz_serializer` should implement the above three methods, namely `get_type_id`, `write`, `read`. In case that the object fields are non-public, you can always define struct `hz_serializer<Person>` as friend to your object class. You use the `object_data_output` class methods when serializing the object into binary bytes and you use the `object_data_input` class methods when de-serializing the bytes into the concrete object instance. 

## 4.4. JSON Serialization

You can use the JSON formatted strings as objects in Hazelcast cluster. Creating JSON objects in the cluster does not require any server side coding and hence you can just send a JSON formatted string object to the cluster and query these objects by fields.

In order to use JSON serialization, you should use the `hazelcast_json_value` object for the key or value. Here is an example imap usage:

```c++
auto hz = hazelcast::new_client().get();

auto map = hz.get_map("map").get();

map->put("item1", hazelcast::client::hazelcast_json_value("{ \"age\": 4 }")).get();
map->put("item2", hazelcast::client::hazelcast_json_value("{ \"age\": 20 }")).get();
```

`hazelcast_json_value` is a simple wrapper and identifier for the JSON formatted strings. You can get the JSON string from the `hazelcast_json_value` object using the `to_string()` method. 

You can construct a `hazelcast_json_value` using one of the constructors. All constructors accept the JSON formatted string as the parameter. No JSON parsing is performed but it is your responsibility to provide correctly formatted JSON strings. The client will not validate the string, and it will send it to the cluster as it is. If you submit incorrectly formatted JSON strings and, later, if you query those objects, it is highly possible that you will get formatting errors since the server will fail to deserialize or find the query fields.

You can query JSON objects in the cluster using the `predicate`s of your choice. An example JSON query for querying the values whose age is less than 6 is shown below:

```c++
// Get the objects whose age is less than 6
auto result = map->values<hazelcast::client::hazelcast_json_value>(
hazelcast::client::query::greater_less_predicate(hz, "age", 6, false, true)).get();
```

## 4.5. Global Serialization

The global serializer is registered as a fallback serializer to handle all other objects if a serializer cannot be located for them.

By default, global serializer is used if there are no specialization `hz_serializer<my_class>` for my class `my_class`. 

**Use Cases:**

* Third party serialization frameworks can be integrated using the global serializer.
* For your custom objects, you can implement a single serializer to handle all object serializations.

A sample global serializer that integrates with a third party serializer is shown below.

```c++
class MyGlobalSerializer : public hazelcast::client::serialization::global_serializer {
public:
    void write(const boost::any &obj, hazelcast::client::serialization::object_data_output &out) override {
        auto const &object = boost::any_cast<Person>(obj);
        out.write(object.name);
        out.write(object.male);
        out.write(object.age);
    }

    boost::any read(hazelcast::client::serialization::object_data_input &in) override {
        return boost::any(Person{in.read<std::string>(), in.read<bool>(), in.read<int32_t>()});
    }
};
```

As you see fromn the sample, the global serializer class should implement the `hazelcast::client::serialization::global_serializer` interface.

You should register the global serializer in the configuration.

```c++
hazelcast::client::client_config config;
config.get_serialization_config().set_global_serializer(std::make_shared<MyGlobalSerializer>());
auto hz = hazelcast::new_client(std::move(config)).get();
```

You need to utilize the `boost::any_cast` methods tyo actually use the objects provided for serialization and you are expected to return type `boost::any` from the `read` method. 

# 5. Setting Up Client Network

All network related configuration of Hazelcast C++ client is performed programmatically via the `client_network_config` object. The following is an example configuration.

Here is an example of configuring the network for C++ Client programmatically.

```c++
client_config clientConfig;
clientConfig.get_network_config().add_addresses({{"10.1.1.21", 5701}, {"10.1.1.22", 5703}});
clientConfig.get_network_config().set_smart_routing(true);
clientConfig.set_redo_operation(true);
clientConfig.get_connection_strategy_config().get_retry_config().set_cluster_connect_timeout(
        std::chrono::seconds(30));
```

## 5.1. Providing Member Addresses

Address list is the initial list of cluster addresses which the client will connect to. The client uses this
list to find an alive member. Although it may be enough to give only one address of a member in the cluster
(since all members communicate with each other), it is recommended that you give the addresses for all the members.

```c++
client_config clientConfig;
clientConfig.get_network_config().add_address(address("10.1.1.21", 5701), address("10.1.1.22", 5703));
```

The provided list is shuffled and tried in a random order. If no address is added to the `client_network_config`, then `127.0.0.1:5701` is tried by default.

## 5.2. Setting Smart Routing

Smart routing defines whether the client mode is smart or unisocket. See the [C++ Client Operation Modes section](#72-c-client-operation-modes)
for the description of smart and unisocket modes.
 
The following is an example configuration.

```c++
client_config clientConfig;
clientConfig.get_network_config().set_smart_routing(true);
```

Its default value is `true` (smart client mode).

## 5.3. Enabling Redo Operation

It enables/disables redo-able operations. While sending the requests to the related members, the operations can fail due to various reasons. Read-only operations are retried by default. If you want to enable retry for the other operations, you can set the `redo_operation` to `true`.

```c++
client_config clientConfig;
clientConfig.set_redo_operation(true);
```

Its default value is `false` (disabled).

## 5.4. Setting Cluster Connection Timeout
Cluster connection timeout is the timeout value for which the client tries to connect to the cluster. If the client can not connect to cluster during this timeout duration, the client shuts down itself and it can not be re-used (you need to obtain a new client).
 
The following example shows how you can set the cluster connection timeout to 30 seconds.

```c++
client_config().get_connection_strategy_config().get_retry_config().set_cluster_connect_timeout(std::chrono::seconds(30));
```

## 5.5. Advanced Cluster Connection Retry Configuration

When client is disconnected from the cluster, it searches for new connections to reconnect. You can configure the frequency of the reconnection attempts and client shutdown behavior using `connection_retry_config`.
The following is an example configuration.

Below is an example configuration. It configures a total timeout of 30 seconds to connect to a cluster, by initial backoff time being 100 milliseconds and doubling the time before every try with a jitter of 0.8  up to a maximum of 3 seconds backoff between each try..

```c++
client_config().get_connection_strategy_config().get_retry_config().set_cluster_connect_timeout(
        std::chrono::seconds(30)).set_multiplier(2.0).set_jitter(0.8).set_initial_backoff_duration(
        std::chrono::seconds(100)).set_max_backoff_duration(std::chrono::seconds(3));
```

The following are configuration element descriptions:

* `initial_backoff_duration`: Specifies how long to wait (backoff) after the first failure before retrying.
* `max_backoff_duration`: Specifies the upper limit for the backoff between each cluster connect tries.
* `multiplier`: Factor to multiply the backoff after a failed retry.
* `cluster_connect)timeout`: Timeout value for the client to give up to connect to the current cluster. If the client can not connect during this time, then it shuts down and it can not be re-used.
* `jitter`: Specifies by how much to randomize backoffs.

## 5.6. Enabling Client TLS/SSL

You can use TLS/SSL to secure the connection between the clients and members. If you want to enable TLS/SSL
for the client-cluster connection, you should set an SSL configuration. Please see the [TLS/SSL section](#61-tlsssl).

As explained in the [TLS/SSL section](#61-tlsssl), Hazelcast members have key stores used to identify themselves (to other members) and Hazelcast C++ clients have certificate authorities used to define which members they can trust. 

## 5.7. Enabling Hazelcast AWS Discovery

The C++ client can discover the existing Hazelcast servers in the Amazon AWS environment. The client queries the Amazon AWS environment using the [describe-instances] (http://docs.aws.amazon.com/cli/latest/reference/ec2/describe-instances.html) query of AWS. The client finds only the up and running instances and filters them based on the filter config provided at the `client_aws_config` configuration.
 
The following is an example configuration:

```c++
clientConfig.get_network_config().get_aws_config()
    .set_enabled(true)
    .set_access_key("my access key id")
    .set_secret_key("my secret access key")
    .set_tag_key("my tag key")
    .set_tag_value("my tag value")
    .set_security_group_name("my secure group")
    .set_region("us-east-1");
```

You need to enable the discovery by calling the `set_enabled(true)`. You can set your access key and secret in the configuration as shown in this example. You can filter the instances by setting which tags they have or by the security group setting. You can set the region for which the instances will be retrieved from, the default region is `us-east-1`.

See [client_aws_config.h](https://github.com/hazelcast/hazelcast-cpp-client/blob/master/hazelcast/include/hazelcast/client/config/client_aws_config.h)
and [the AWS examples directory](https://github.com/hazelcast/hazelcast-cpp-client/tree/master/examples/aws)
for more details.

## 5.8. Enabling Hazelcast Cloud Discovery
If you are using [Hazelcast Cloud Service](https://cloud.hazelcast.com) and you want to write an application that will utilize the Hazelcast cloud database service, you can use the C++ client. The configuration is very simple, you just need to set the cluster name, discovery token and enable the cloud discovery on network settings. Here is an example configuration:

```c++
    hazelcast::client::client_config config;
    config.set_cluster_name("my_cluster(should match with the same cluster name at the cloud web site)");
    auto &cloud_configuration = config.get_network_config().get_cloud_config();
    cloud_configuration.enabled = true;
    cloud_configuration.discovery_token = "My token for the cluster (you can get from the cloud web site)";
    auto hz = hazelcast::new_client(std::move(config)).get();
```

That is all the configuration you will need. The client will query the hazelcast coordinator service located at "https://coordinator.hazelcast.cloud/cluster/discovery?token=my_token" with the token you configured and will get a list of available server ip port combinations to connect to. The client will always use the public ip to connect to the cluster. The network connection timeout (`client_network_config::set_connection_timeout(const std::chrono::milliseconds &timeout)`) is used as the timeout value for connecting the coordinator and retrieving the cluster member list (change this setting if you have any timeout problems).

Please check the code sample at `examples/cloud-discovery/connect-cloud.cpp` for a full featured cloud discovery example.

## 5.8.1. Cloud Discovery With SSL Enabled
You can create a Hazelcast cluster in the cloud with "[Enable Encryption](https://docs.hazelcast.com/cloud/encryption.html#setting-up-encryption)" option. When this option is selected the cluster requires the clients to connect using the SSL connection and the client should be configured to do [mutual authentication](#6122-mutual-authentication-two-way-authentication). The required certificate authority file, client certificate, client key file and key file PEM pass phrase are located at the Hazelcast cloud web site cluster configuration `Configure Clients` page. Download the keystore file and unzip it (it will be folder such as `hzcloud_xxx_keys` where `xxx` is the cluster number), this zip includes all the required files. Also, copy the `Keystore and truststore password` which is the client key PEM file pass phrase. Once, you have all this information in hand, you can configure the client as in the following code snippet:
```c++
    std::string cluster_name = "my_cluster";
    std::string cloud_token = "my cloud token for the cluster";
    std::string ca_file_path = "/path/to/hzcloud_xxx_keys/ca.pem";
    std::string client_certificate = "/path/to/hzcloud_xxx_keys/cert.pem";
    std::string client_key = "/path/to/hzcloud_xxx_keys/key.pem";
    std::string client_key_pem_pass_phrase = "Keystore and truststore password";

    hazelcast::client::client_config config;
    config.set_cluster_name(cluster_name);
    auto &cloud_configuration = config.get_network_config().get_cloud_config();
    cloud_configuration.enabled = true;
    cloud_configuration.discovery_token = cloud_token;

    // ssl configuration
    boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12);
    ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    ctx.load_verify_file(ca_file_path);
    ctx.use_certificate_file(client_certificate, boost::asio::ssl::context::pem);
    ctx.set_password_callback([&] (std::size_t max_length, boost::asio::ssl::context::password_purpose purpose) {
        return client_key_pem_pass_phrase;
    });
    ctx.use_private_key_file(client_key, boost::asio::ssl::context::pem);
    config.get_network_config().get_ssl_config().set_context(std::move(ctx));

    auto hz = hazelcast::new_client(std::move(config)).get();
```

As you see in the code snippet, we provide a `boost::asio::ssl::context` in the client configuration in addition to the cloud token and cluster name configurations. It uses mutual authentication (two-way handshake).

Please check the code sample at `examples/cloud-discovery/ssl-connect-cloud.cpp` for a full featured cloud discovery with SSL example.

## 5.9. External Smart Client Discovery

> **NOTE: This feature requires Hazelcast IMDG 4.2 or higher version.**

The client sends requests directly to cluster members in the smart client mode (default) in order to reduce hops to 
accomplish operations. Because of that, the client should know the addresses of members in the cluster.

In cloud-like environments, or Kubernetes, there are usually two network interfaces: the private and public network 
interfaces. When the client is in the same network as the members, it uses their private network addresses. Otherwise, 
if the client and the Hazelcast cluster are on different networks, the client cannot connect to members using their 
private network addresses. Hazelcast 4.2 introduced External Smart Client Discovery to solve that issue. The client 
needs to communicate with all cluster members via their public IP addresses in this case. Whenever Hazelcast cluster 
members are able to resolve their own public external IP addresses, they pass this information to the client. 
As a result, the client can use public addresses for communication.

In order to use this feature, make sure your cluster members are accessible from the network the client resides in, 
then set config `client_network_config()::use_public_address(true)` to true. You should specify the public address of 
at least one member in the configuration:

```c++
    hazelcast::client::client_config config;
    constexpr int server_port = 5701;
    constexpr const char *server_public_address = "myserver.publicaddress.com";
    config.get_network_config().use_public_address(true).add_address(
            hazelcast::client::address{server_public_address, server_port});
```

This solution works everywhere without further configuration (Kubernetes, AWS, GCP, Azure, etc.) as long as the 
corresponding plugin is enabled in Hazelcast server configuration.

## 5.10. Authentication

By default, the client does not use any authentication method and just uses the cluster "dev" to connect to. You can change which cluster to connect by using the configuration API `client_config::set_cluster_name`. This way, you can have multiple clusters in the network but the client will only be able to connect to the cluster with the correct cluster name (server should also be started with the cluster name configured to the same cluster name).

If you want to enable authentication, then the client can be configured in one of the two different ways to authenticate:

* Username password based authentication
* Token based authentication

## 5.10.1. Username Password Authentication

The following is an example configuration where we set the username `test-user` and password `test-pass` to be used while trying to connect to the cluster:

```c++
hazelcast::client::client_config config;
config.set_credentials(std::make_shared<hazelcast::client::security::username_password_credentials>("test-user", "test-pass"));
auto hz = hazelcast::new_client(std::move(config)).get();
```

Note that the server needs to be configured to use the same username and password. An example server xml config is:
```xml
    <security enabled="true">
        <realms>
            <realm name="usernamePasswordIdentityRealm">
                <authentication>
                    <simple>
                        <user username="test-user" password="test-pass"/>
                    </simple>
                </authentication>
            </realm>
        </realms>
        <client-authentication realm="usernamePasswordIdentityRealm"/>
    </security>
```

## 5.10.2. Token Authentication

The following is an example configuration where we set the secret token bytes to be used while trying to connect to the cluster:

```c++
std::vector<hazelcast::byte> my_token = {'S', 'G', 'F', '6', 'Z', 'W'};
config.set_credentials(std::make_shared<hazelcast::client::security::token_credentials>(my_token));
auto hz = hazelcast::new_client(std::move(config)).get();
```
Implementing a custom login module is mandatory to use token authentication by itself. An example custom login module is:
```Java
import com.hazelcast.config.Config;
import com.hazelcast.security.ClusterLoginModule;
import com.hazelcast.security.Credentials;
import com.hazelcast.security.CredentialsCallback;
import com.hazelcast.security.TokenCredentials;
import javax.security.auth.callback.Callback;
import javax.security.auth.callback.UnsupportedCallbackException;
import javax.security.auth.login.FailedLoginException;
import javax.security.auth.login.LoginException;
import java.io.IOException;
import java.util.Arrays;

public class TokenLoginModule extends ClusterLoginModule {

    private String name;
    private final byte[] expectedToken = "SGF6ZW".getBytes(StandardCharsets.US_ASCII);

    @Override
    protected boolean onLogin() throws LoginException {
        CredentialsCallback cb = new CredentialsCallback();
	name = cb.getCredentials().getName();
        try {
            callbackHandler.handle(new Callback[] { cb });
        } catch (IOException | UnsupportedCallbackException e) {
            throw new LoginException("Problem getting credentials");
        }
        Credentials credentials = cb.getCredentials();
        if (!(credentials instanceof TokenCredentials)) {
            throw new FailedLoginException();
        }
        byte[] actualToken = ((TokenCredentials) credentials).getToken();
        if (Arrays.equals(actualToken, expectedToken)) {
            return true;
        }
        throw new LoginException("Invalid token");
    }

    @Override
    protected String getName() {
        return name;
    }

}
```

Note that the server needs to be configured to use the same token. An example server xml config is:
```xml
    <security enabled="true">
    <realms>
        <realm name="jaasRealm">
            <authentication>
                <jaas>
                    <login-module class-name="com.example.TokenLoginModule" usage="REQUIRED"/>
                </jaas>
            </authentication>
        </realm>
    </realms>
    <client-authentication realm="jaasRealm"/>
</security>
```

## 5.11. Configuring Backup Acknowledgment

When an operation with sync backup is sent by a client to the Hazelcast member(s), the acknowledgment of the operation's backup is sent to the client by the backup replica member(s). This improves the performance of the client operations.

To disable backup acknowledgement, you should use the `backup_acks_enabled` configuration option.

```c++
// Disable the default backup ack feature
hazelcast_client hz(client_config().backup_acks_enabled(false));
```

Its default value is `true`. This option has no effect for unisocket clients.

You can also fine-tune this feature using `client_config::set_protperty` API as described below:

- `hazelcast.client.operation.backup.timeout.millis`: Default value is `5000` milliseconds. If an operation has backups, this property specifies how long (in milliseconds) the invocation waits for acks from the backup replicas. If acks are not received from some of the backups, there will not be any rollback on the other successful replicas.

- `hazelcast.client.operation.fail.on.indeterminate.state`: Default value is `false`. When it is `true`, if an operation has sync backups and acks are not received from backup replicas in time, or the member which owns primary replica of the target partition leaves the cluster, then the invocation fails. However, even if the invocation fails, there will not be any rollback on other successful replicas.

# 6. Securing Client Connection

This chapter describes the security features of Hazelcast C++ client. These include using TLS/SSL for connections between members and between clients and members. These security features require **Hazelcast IMDG Enterprise** server cluster.

## 6.1. TLS/SSL

One of the offers of Hazelcast is the TLS/SSL protocol which you can use to establish an encrypted communication between the client and the server.

### 6.1.1. TLS/SSL for Hazelcast Members

Hazelcast allows you to encrypt socket level communication between Hazelcast members and between Hazelcast clients and members, for end to end encryption. To use it, see the [TLS/SSL for Hazelcast Members section](https://docs.hazelcast.com/imdg/latest/security/tls-ssl.html#tlsssl-for-hazelcast-members) in the Hazelcast IMDG Reference Manual.

### 6.1.2. TLS/SSL for Hazelcast C++ Client

The Hazelcast C++ client uses [Boost Asio](https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio.html) library for networking and secure communication. 

To use TLS/SSL with your Hazelcast C++ client, you should build the library with OpenSSL feature turned on. By default, this feature is turned off. Here are the different ways to install the library with SSL support:

* CMake Users: Provide flag `-DWITH_OPENSSL=ON` flag when configuring. Please note that the CMake `find_package` should be able to locate the OpenSSL installation, otherwise the cmake config will fail. The rest is as usual with the [installation with cmake](#131-cmake-users).
* Conan Users: Use option `with_openssl` while installing the conan project. An example command you can use in the build folder of your project with conan: `conan install -o hazelcast-cpp-client:with_openssl=True .. --build=hazelcast-cpp-client`. This command will install the `hazelcast-cpp-client` with the OpenSSL support.

Once the library is installed properly, you can enable the SSL feature in the client configuration.  

You can set the protocol type. If not set, the configuration uses `tlsv12` (TLSv1.2) as the default protocol type and version.

There may be different ways to configure ssl communication at the client side. We utilize the `boost::asio::ssl::context` to configure the SSL communication. You can set the verify mode to ignore or verify the server certificate, set the used SSL protocol, add verify callbacks, and such. The details can be found at the [Boost asio ssl context documentation](https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio/reference/ssl__context.html).

Here is the most basic way to enable the SSL on the client:

```c++
hazelcast::client::client_config config;
boost::asio::ssl::context ctx(boost::asio::ssl::context::method::tlsv12_client);
config.get_network_config().get_ssl_config().set_context(std::move(ctx));
auto hz = hazelcast::new_client(std::move(config)).get();
```

Once, we set the SSL context, the client will start with SSL enabled and try to connect to the server using the provided context.

#### 6.1.2.1. Enabling Validation Of Server Certificate
If you want to enable the validation of server certificate on the client side and disable any connection without a valid certificate, you can use the [context::set_verify_mode](https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio/reference/ssl__context/set_verify_mode.html) API. Here is an example:

```c++
    hazelcast::client::client_config config;

    boost::asio::ssl::context ctx(boost::asio::ssl::context::method::tlsv12_client);
    ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    ctx.set_default_verify_paths();

    config.get_network_config().get_ssl_config().set_context(std::move(ctx));    
    auto hz = hazelcast::new_client(std::move(config)).get();
```

The client will connect if the server has a certificate which is properly signed by well known Certificate Authorities(CA). On Windows, you may need to set the well known CA Authorities file path since OpenSSL may not find the well known certifite authorities file. This can be solved by providing the [SSL_CERT_FILE](https://www.openssl.org/docs/man1.1.0/man3/SSL_CTX_set_default_verify_paths.html) environment variable set to point a file path that has all the needed well known certificate authorities. For example, you can get such a file maintained by Mozilla at https://curl.se/docs/caextract.html.

If the server is using a user generated certificate file which is not signed by the well known CA authorities, then you can use the public certificate of the server to configure the client to connect and verify the server. Here is an example configuration for this:

```c++
    hazelcast::client::client_config config;

    boost::asio::ssl::context ctx(boost::asio::ssl::context::method::tlsv12_client);
    ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    ctx.set_default_verify_paths();
    ctx.load_verify_file("/path/to/my/server/public/certificate");

    config.get_network_config().get_ssl_config().set_context(std::move(ctx));
    auto hz = hazelcast::new_client(std::move(config)).get();
```

As you can see in this code snippet, we add the server public certificate using [context::load_verify_file](https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio/reference/ssl__context/load_verify_file.html) as a verify file so that the client can validate the server certificate as valid and connect to the server. There are also other ways to load verify files and use verify directories. You can find the details at [Boost Asio ssl context documentation](https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio/reference/ssl__context.html). 

You can check `BasicTLSClient.cpp` example for a full featured example.

#### 6.1.2.2. Mutual Authentication (Two Way Authentication) 
[Mutual Authentication](https://en.wikipedia.org/wiki/Mutual_authentication) is the process where the client verifies the identity of the server via server's certificate (either self-signed or signed by a CA authority) and the server verifies the client identity via the client provided certificate (either self-signed or signed by a CA authority). If the Hazelcast server is configured for [mutual authentication](https://docs.hazelcast.com/imdg/latest/security/tls-ssl.html#tlsssl-for-hazelcast-members) as **REQUIRED**, then we can use the [ssl::context::use_xxx](https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio/reference/ssl__context.html) methods to add the client's public and private certificates and use them during its authentication to the server. An example configuration is as follows:
```c++
    hazelcast::client::client_config config;

    boost::asio::ssl::context ctx(boost::asio::ssl::context::method::tlsv12_client);
    ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    ctx.set_default_verify_paths();
    
    // This config is to validate the server certificate if server does not have a CA signed certificate
    ctx.load_verify_file("/path/to/my/server/public/certificate");
    
    // The following two lines configure the client to use the client certificate to introduce itself to the server
    ctx.use_certificate_file("/path/to/my/client/public/certificate", boost::asio::ssl::context::pem);
    ctx.use_private_key_file("/path/to/my/client/private/certificate", boost::asio::ssl::context::pem);

    config.get_network_config().get_ssl_config().set_context(std::move(ctx));
    auto hz = hazelcast::new_client(std::move(config)).get();
```

With the above configuration, both the client and server authenticate each other mutually. The client validates the server certificate and the server validates the client with the client certificate.

You can check `mutual_authentication.cpp` for a full featured example.

#### 6.1.2.3. Constraining The Used Cipher List
In some cases, you may want to limit the cipher suites allowed for a client while communicating with the server. This can also be configured using `ssl_config`. Here is an example configuration to enable cipher list which allow only "HIGH" ciphers.
```c++
    config.get_network_config().get_ssl_config().
            set_cipher_list("HIGH");     // optional setting (values for string are described at
                                                // https://www.openssl.org/docs/man1.0.2/apps/ciphers.html and
                                                // https://www.openssl.org/docs/man1.1.1/man1/ciphers.html
```

Cipher list string format and the details can be found at the official [OpenSSL Documentation](https://www.openssl.org/docs/man1.1.1/man1/ciphers.html).

# 7. Using C++ Client with Hazelcast IMDG

This chapter provides information on how you can use Hazelcast IMDG's data structures in the C++ client, after giving some basic information including an overview to the client API, operation modes of the client and how it handles the failures.

## 7.1. C++ Client API Overview

This chapter provides information on how you can use Hazelcast IMDG's data structures in the C++ client, after giving some basic information including an overview to the client API, operation modes of the client and how it handles the failures.

Most of the methods in C++ API are synchronous. The failures are communicated via exceptions. All exceptions are derived from the `hazelcast::client::exception::iexception` base method. There are also asynchronous versions of some methods in the API. The asynchronous ones use the `hazelcast::client::Future<T>` future object. It works similar to the `std::future`.

If you are ready to go, let's start to use Hazelcast C++ client!

The first step is the configuration. You can configure the C++ client programmatically. See the [Programmatic Configuration section](#311-programmatic-configuration) for details. 

The following is an example on how to create a `client_config` object and configure it programmatically:

```c++
client_config clientConfig;
clientConfig.set_cluster_name("my test cluster");
clientConfig.get_network_config().add_addresses({{"'10.90.0.1", 5701}, {"'10.90.0.2", 5702}});
```

The second step is initializing the `hazelcast_client` to be connected to the cluster:

```c++
auto client = new_client(std::move(clientConfig));
// some operation
```

**This client object is your gateway to access all the Hazelcast distributed objects.**

Let's create a map and populate it with some data, as shown below.

```c++
// Get the Distributed Map from Cluster.
auto map = client.get_map("my-distributed-map").get();
//Standard put and get.
map->put<std::string, std::string>("key", "value").get();
map->get<std::string, std::string>("key").get();
//Concurrent Map methods, optimistic updating
map->put_if_absent<std::string, std::string>("somekey", "somevalue").get();
map->replace<std::string, std::string>("key", "value", "newvalue").get();
 ```

As the final step, if you are done with your client, you can shut it down as shown below. This will release all the used resources and close connections to the cluster.

```c++
// Shutdown this Hazelcast Client
client.shutdown().get();
```

The client object destructor also shuts down the client upon destruction if you do not explicitly call the shutdown method.

## 7.2. C++ Client Operation Modes

The client has two operation modes because of the distributed nature of the data and cluster: smart and unisocket.

### 7.2.1. Smart Client

In the smart mode, the clients connect to each cluster member. Since each data partition uses the well known and consistent hashing algorithm, each client can send an operation to the relevant cluster member, which increases the overall throughput and efficiency. Smart mode is the default mode.

### 7.2.2. Unisocket Client

For some cases, the clients can be required to connect to a single member instead of each member in the cluster. Firewalls, security or some custom networking issues can be the reason for these cases.

In the unisocket client mode, the client will only connect to one of the configured addresses. This single member will behave as a gateway to the other members. For any operation requested from the client, it will redirect the request to the relevant member and return the response back to the client returned from this member.

You can set the unisocket client mode in the `client_config` as shown below.

```c++
clientConfig.get_network_config().set_smart_routing(false);
```

## 7.3. Handling Failures

There are two main failure cases you should be aware of. Below sections explain these and the configurations you can perform to achieve proper behavior.

### 7.3.1. Handling Client Connection Failure

While the client is trying to connect initially to one of the members in the `client_network_config::get_addresses()`, all the members might not be available. Instead of giving up, throwing an error and stopping the client, the client retries to connect as configured which is described in the [Advanced Cluster Connection Retry Configuration](#55-advanced-cluster-connection-retry-configuration) section.

The client executes each operation through the already established connection to the cluster. If this connection(s) disconnects or drops, the client tries to reconnect as configured.

### 7.3.2. Handling Retry-able Operation Failure

While sending the requests to the related members, the operations can fail due to various reasons. Read-only operations are retried by default. If you want to enable retrying for the other operations, you can set the `redo_operation` to `true`. See the [Enabling Redo Operation section](#53-enabling-redo-operation).

You can set a timeout for retrying the operations sent to a member. This can be provided by using the property `hazelcast.client.invocation.timeout.seconds` in `client_config.properties`. The client will retry an operation within this given period, of course, if it is a read-only operation or you enabled the `redo_operation` as stated in the above paragraph. This timeout value is important when there is a failure resulted by either of the following causes:

* Member throws an exception.
* Connection between the client and member is closed.
* Client’s heartbeat requests are timed out.

When a connection problem occurs, an operation is retried if it is certain that it has not run on the member yet or if it is idempotent such as a read-only operation, i.e., retrying does not have a side effect. If it is not certain whether the operation has run on the member, then the non-idempotent operations are not retried. However, as explained in the first paragraph of this section, you can force all the client operations to be retried (`redo_operation`) when there is a connection failure between the client and member. But in this case, you should know that some operations may run multiple times causing conflicts. For example, assume that your client sent a `queue->offer` operation to the member and then the connection is lost. Since there will be no response for this operation, you will not know whether it has run on the member or not. If you enabled `redo_operation`, it means this operation may run again, which may cause two instances of the same object in the queue.

When invocation is being retried, the client may wait some time before it retries again. This duration can be configured using the following property:

```
config.set_property(“hazelcast.client.invocation.retry.pause.millis”, “500");
```
The default retry wait time is 1 second.

### 7.3.3. Client Backpressure

Hazelcast uses operations to make remote calls. For example, a `map->get` is an operation and a `map->put` is one operation for the primary
and one operation for each of the backups, i.e., `map->put` is executed for the primary and also for each backup. In most cases, there will be a natural balance between the number of threads performing operations
and the number of operations being executed. However, there are two situations where this balance and operations
can pile up and eventually lead to `out_of_memory_exception` (OOME):

- Asynchronous calls: With async calls, the system may be flooded with the requests.
- Asynchronous backups: The asynchronous backups may be piling up.

To prevent the system from crashing, Hazelcast provides back pressure. Back pressure works by:

- limiting the number of concurrent operation invocations,
- periodically making an async backup sync.

Sometimes, e.g., when your servers are overloaded, you may want to slow down the client operations to the cluster. Then the client can be configured to wait until number of outstanding invocations whose responses are not received to become less than a certain number. This is called Client Back Pressure. By default, the backpressure is disabled. There are a few properties which control the back pressure. The following are these client configuration properties:

- `hazelcast.client.max.concurrent.invocations`: The maximum number of concurrent invocations allowed. To prevent the system from overloading, you can apply a constraint on the number of concurrent invocations. If the maximum number of concurrent invocations has been exceeded and a new invocation comes in, then Hazelcast will throw `hazelcast_overload`. By default this property is configured as INT32_MAX.
- `hazelcast.client.invocation.backoff.timeout.millis`: Controls the maximum timeout in milliseconds to wait for an invocation space to be available. If an invocation can't be made because there are too many pending invocations, then an exponential backoff is done to give the system time to deal with the backlog of invocations. This property controls how long an invocation is allowed to wait before getting `hazelcast_overload`. When set to -1 then `hazelcast_overload` is thrown immediately without any waiting. This is the default value.

For details of backpressure, see the [Back Pressure section](https://docs.hazelcast.com/imdg/latest/performance/back-pressure.html) in the Hazelcast IMDG Reference Manual.

## 7.3.4 Client Connection Strategy

Hazelcast client-cluster connection and reconnection strategy can be configured. Sometimes, you may not want your application to wait for the client to connect to the cluster, you may just want to get the client and let the client connect in the background. This is configured as follows:

```
client_config::get_connection_strategy_config().set_async_start(bool);
```

When this configuration is set to true, the client creation won't wait to connect to cluster. The client instance will throw an exception for any request, until it connects to the cluster and become ready.

If it is set to false (the default case), `hazelcast_client(const client_config)` will block until a cluster connection is established and it's ready to use the client instance.

### 7.3.4.1 Configuring Client Reconnect Strategy

You can configure how the client should act when the client disconnects from the cluster for any reason. This is configured as follows:

```
client_config::get_connection_strategy_config().set_reconnect_mode(const hazelcast::client::config::client_connection_strategy_config::reconnect_mode &);
```

Possible values for `reconnect_mode` are:

- `OFF`: Prevents reconnection to the cluster after a disconnect.
- `ON`: Reconnects to the cluster by blocking invocations.
- `ASYNC`: Reconnects to the cluster without blocking invocations. Invocations will receive `hazelcast_client_offline`.

## 7.4. Using Distributed Data Structures

Most of the distributed data structures are supported by the C++ client. In this chapter, you will learn how to use these distributed data structures.

### 7.4.1. Using imap

Hazelcast Map (`imap`) is a distributed map. Through the C++ client, you can perform operations like reading and writing from/to a Hazelcast Map with the well known get and put methods. For details, see the [Map section](https://docs.hazelcast.com/imdg/latest/data-structures/map.html) in the Hazelcast IMDG Reference Manual.

A Map usage example is shown below.

```c++
// Get the Distributed Map from Cluster.
auto map = hz.get_map("my-distributed-map").get();
//Standard Put and Get.
map->put<std::string, std::string>("key", "value").get();
map->get<std::string, std::string>("key").get();
//Concurrent Map methods, optimistic updating
map->put_if_absent<std::string, std::string>("somekey", "somevalue").get();
map->replace<std::string, std::string>("key", "value", "newvalue").get();
```

### 7.4.2. Using multi_map

Hazelcast `multi_map` is a distributed and specialized map where you can store multiple values under a single key. For details, see the [MultiMap section](https://docs.hazelcast.com/imdg/latest/data-structures/multimap.html) in the Hazelcast IMDG Reference Manual.

A multi_map usage example is shown below.

```c++
// Get the Distributed multi_map from Cluster.
auto multiMap = hz.get_multi_map("my-distributed-multimap").get();
// Put values in the map against the same key
multiMap->put<std::string, std::string>("my-key", "value1").get();
multiMap->put<std::string, std::string>("my-key", "value2").get();
multiMap->put<std::string, std::string>("my-key", "value3").get();
// Print out all the values for associated with key called "my-key"
for (const auto &value : multiMap->get<std::string, std::string>("my-key").get()) {
    std::cout << value << std::endl; // will print value1, value2 and value3 each per line.
}
// remove specific key/value pair
multiMap->remove<std::string, std::string>("my-key", "value2").get();
```

### 7.4.3. Using replicated_map

Hazelcast `replicated_map` is a distributed key-value data structure where the data is replicated to all members in the cluster. It provides full replication of entries to all members for high speed access. For details, see the [ReplicatedMap section](https://docs.hazelcast.com/imdg/latest/data-structures/replicated-map.html) in the Hazelcast IMDG Reference Manual.

A replicated_map usage example is shown below.

```c++
auto replicatedMap = hz.get_replicated_map("myReplicatedMap").get();
replicatedMap->put<int, std::string>(1, "Furkan").get();
replicatedMap->put<int, std::string>(2, "Ahmet").get();
std::cout << "Replicated map value for key 2 is " << *replicatedMap->get<int, std::string>(2).get() << std::endl; // Replicated map value for key 2 is Ahmet
```

### 7.4.4. Using iqueue

Hazelcast Queue (`iqueue`) is a distributed queue which enables all cluster members to interact with it. For details, see the [Queue section](https://docs.hazelcast.com/imdg/latest/data-structures/queue.html) in the Hazelcast IMDG Reference Manual.

A Queue usage example is shown below.

```c++
// Get a Blocking Queue called "my-distributed-queue"
auto queue = hz.get_queue("my-distributed-queue").get();
// Offer a String into the Distributed Queue
queue->offer<std::string>("item").get();
// Poll the Distributed Queue and return the String
auto item = queue->poll<std::string>().get(); // gets first "item"
//Timed blocking Operations
queue->offer<std::string>("anotheritem", 500).get();
auto anotherItem = queue->poll<std::string>(5 * 1000).get();
//Indefinitely blocking Operations
queue->put<std::string>("yetanotheritem").get();
std::cout << *queue->take<std::string>().get() << std::endl; // Will print yetanotheritem
```

### 7.4.5. Using iset

Hazelcast Set (`iset`) is a distributed set which does not allow duplicate elements. For details, see the [Set section](https://docs.hazelcast.com/imdg/latest/data-structures/set.html) in the Hazelcast IMDG Reference Manual.

A Set usage example is shown below.

```c++
// Get the Distributed Set from Cluster.
auto set = hz.get_set("my-distributed-set").get();
// Add items to the set with duplicates
set->add<std::string>("item1").get();
set->add<std::string>("item1").get();
set->add<std::string>("item2").get();
set->add<std::string>("item2").get();
set->add<std::string>("item2").get();
set->add<std::string>("item3").get();
// Get the items. Note that there are no duplicates.
for (const auto &value : set->toArray().get()) {
    std::cout << value << std::endl;
}
```

### 7.4.6. Using ilist

Hazelcast List (`ilist`) is a distributed list which allows duplicate elements and preserves the order of elements. For details, see the [List section](https://docs.hazelcast.com/imdg/latest/data-structures/list.html) in the Hazelcast IMDG Reference Manual.

A List usage example is shown below.

```c++
// Get the Distributed List from Cluster.
auto list = hz.get_list("my-distributed-list").get();
// Add elements to the list
list->add<std::string>("item1").get();
list->add<std::string>("item2").get();

// Remove the first element
std::cout << "Removed: " << *list.remove<std::string>(0).get();
// There is only one element left
std::cout << "Current size is " << list.size().get() << std::endl;
// Clear the list
list.clear().get();
```

### 7.4.7. Using ringbuffer

Hazelcast `ringbuffer` is a replicated but not partitioned data structure that stores its data in a ring-like structure. You can think of it as a circular array with a given capacity. Each ringbuffer has a tail and a head. The tail is where the items are added and the head is where the items are overwritten or expired. You can reach each element in a ringbuffer using a sequence ID, which is mapped to the elements between the head and tail (inclusive) of the ringbuffer. For details, see the [Ringbuffer section](https://docs.hazelcast.com/imdg/latest/data-structures/ringbuffer.html) in the Hazelcast IMDG Reference Manual.

A ringbuffer usage example is shown below.

```c++
auto rb = hz.get_ring_buffer("rb").get();
// add two items into ring buffer
rb->add(100).get();
rb->add(200).get();
// we start from the oldest item.
// if you want to start from the next item, call rb.tailSequence()+1
auto sequence = rb->head_sequence().get();
std::cout << *rb->read_one<int>(sequence).get() << std::endl; //
sequence++;
std::cout << *rb->read_one<int>(sequence).get() << std::endl;
```

### 7.4.8. Using reliable_topic

Hazelcast `reliable_topic` is a distributed topic implementation backed up by the `ringbuffer` data structure. For details, see the [Reliable Topic section](https://docs.hazelcast.com/imdg/latest/data-structures/reliable-topic.html) in the Hazelcast IMDG Reference Manual.

A reliable_topic usage example is shown below.

```c++
hazelcast::client::topic::reliable_listener make_listener(std::atomic<int> &n_received_messages, int64_t sequence_id = -1) {
  using namespace hazelcast::client::topic;
  
  return reliable_listener(false, sequence_id).on_received([&n_received_messages](message &&message){
      ++n_received_messages;
      auto object = message.get_message_object().get<std::string>();
      if (object) {
        std::cout << "[GenericListener::onMessage] Received message: " << *object << " for topic:" << message.get_name() << std::endl;
      } else {
        std::cout << "[GenericListener::onMessage] Received message with NULL object for topic:" << message.get_name() << std::endl;
      }
  });
}

void listen_with_default_config() {
  auto client = hazelcast::new_client().get();
  
  std::string topicName("MyReliableTopic");
  auto topic = client.get_reliable_topic(topicName).get();
  
  std::atomic<int> numberOfMessagesReceived{ 0 };
  auto listenerId = topic->add_message_listener(make_listener(numberOfMessagesReceived));
  
  std::cout << "Registered the listener with listener id:" << listenerId << std::endl;
  
  while (numberOfMessagesReceived < 1) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  
  if (topic->remove_message_listener(listenerId)) {
      std::cout << "Successfully removed the listener " << listenerId << " for topic " << topicName << std::endl;
  } else {
      std::cerr << "Failed to remove the listener " << listenerId << " for topic " << topicName << std::endl;
  }
}

void listen_with_config() {
  hazelcast::client::client_config clientConfig;
  std::string topicName("MyReliableTopic");
  hazelcast::client::config::reliable_topic_config reliableTopicConfig(topicName.c_str());
  reliableTopicConfig.set_read_batch_size(5);
  clientConfig.add_reliable_topic_config(reliableTopicConfig);
  auto client = hazelcast::new_client(std::move(clientConfig)).get();
  
  auto topic = client.get_reliable_topic(topicName).get();
  
  std::atomic<int> numberOfMessagesReceived{ 0 };
  auto listenerId = topic->add_message_listener(make_listener(numberOfMessagesReceived));
  
  std::cout << "Registered the listener with listener id:" << listenerId << std::endl;
  
  while (numberOfMessagesReceived < 1) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  
  if (topic->remove_message_listener(listenerId)) {
    std::cout << "Successfully removed the listener " << listenerId << " for topic " << topicName << std::endl;
  } else {
    std::cerr << "Failed to remove the listener " << listenerId << " for topic " << topicName << std::endl;
  }
}
```

### 7.4.9 Using pn_counter

Hazelcast `pn_counter` (Positive-Negative Counter) is a CRDT positive-negative counter implementation. It is an eventually consistent counter given there is no member failure. For details, see the [PN Counter section](https://docs.hazelcast.com/imdg/latest/data-structures/pn-counter.html) in the Hazelcast IMDG Reference Manual.

A pn_counter usage example is shown below.

```c++
auto hz = hazelcast::new_client().get();

auto pnCounter = hz.get_pn_counter("pncounterexample").get();

std::cout << "Counter started with value:" << pnCounter->get().get() << std::endl;

std::cout << "Counter new value after adding is: " << pnCounter->add_and_get(5).get() << std::endl;

std::cout << "Counter new value before adding is: " << pnCounter->get_and_add(2).get() << std::endl;

std::cout << "Counter new value is: " << pnCounter->get().get() << std::endl;

std::cout << "Decremented counter by one to: " << pnCounter->decrement_and_get().get() << std::endl;
```

### 7.4.10 Using flake_id_generator

Hazelcast `flake_id_generator` is used to generate cluster-wide unique identifiers. Generated identifiers are long primitive values and are k-ordered (roughly ordered). IDs are in the range from 0 to `2^63-1` (maximum signed long value). For details, see the [Flake ID Generator section](https://docs.hazelcast.com/imdg/latest/data-structures/flake-id-generator.html) in the Hazelcast IMDG Reference Manual.

A flake_id_generator usage example is shown below.

```c++
auto generator = hz.get_flake_id_generator("flakeIdGenerator").get();
std::cout << "Id : " << generator->newId().get() << std::endl; // Id : <some unique number>
```

### 7.4.11. CP Subsystem

Hazelcast IMDG 4.0 introduced CP concurrency primitives with respect to the [CAP principle](http://awoc.wolski.fi/dlib/big-data/Brewer_podc_keynote_2000.pdf), i.e., they always maintain [linearizability](https://aphyr.com/posts/313-strong-consistency-models) and prefer consistency over availability during network partitions and client or server failures.

All data structures within CP Subsystem are available through the `hazelcast_client::get_cp_subsystem()` API.

Before using the CP structures, CP Subsystem has to be enabled on the cluster-side. Refer to the [CP Subsystem](https://docs.hazelcast.com/imdg/latest/cp-subsystem/cp-subsystem.html) documentation for more information.

Data structures in CP Subsystem run in CP groups. Each CP group elects its own Raft leader and runs the Raft consensus algorithm independently. The CP data structures differ from the other Hazelcast data structures in two aspects. First, an internal commit is performed on the METADATA CP group every time you fetch a proxy from this interface. Hence, callers should cache the returned proxy objects. Second, if you call `DistributedObject::destroy()` on a CP data structure proxy, that data structure is terminated on the underlying CP group and cannot be reinitialized until the CP group is force-destroyed. For this reason, please make sure that you are completely done with a CP data structure before destroying its proxy.

#### 7.4.11.1. Using atomic_long

Hazelcast `atomic_long` is the distributed implementation of atomic 64-bit integer counter. It offers various atomic operations such as `get`, `set`, `get_and_set`, `compare_and_set`, `increment_and_get`. This data structure is a part of CP Subsystem.

An Atomic Long usage example is shown below.

```c++
// also present the future continuation capability. you can use sync version as well.
// Get current value (returns a int64_t)
auto future = atomic_counter->get().then(boost::launch::deferred, [=] (boost::future<int64_t> f) {
        // Prints:
        // Value: 0
        std::cout << "Value: " << f.get() << "\n";

        // Increment by 42
        auto value = atomic_counter->add_and_get(42).get();
        std::cout << "New value: " << value << "\n";

        // Set to 0 atomically if the current value is 42
        auto result = atomic_counter->compare_and_set(42, 0).get();
        std::cout << "result: " << value << "\n";
        // Prints:
        // CAS operation result: 1
});

future.get();
```

atomic_long implementation does not offer exactly-once / effectively-once execution semantics. It goes with at-least-once execution semantics by default and can cause an API call to be committed multiple times in case of CP member failures. It can be tuned to offer at-most-once execution semantics. Please see the [`fail-on-indeterminate-operation-state`](https://docs.hazelcast.com/imdg/latest/cp-subsystem/configuration.html) server-side setting.

#### 7.4.11.2. Using fenced_lock

Hazelcast `fenced_lock` is the distributed implementation of a linearizable lock. It offers multiple  operations for acquiring the lock. This data structure is a part of CP Subsystem.

A basic Lock usage example is shown below.

```c++
// Get an fenced_lock named 'my-lock'
auto lock = hz.get_cp_subsystem().get_lock("my-lock").get();

// Acquire the lock
lock->lock().get();
try {
    // Your guarded code goes here
} catch (...) {
    // Make sure to release the lock
    lock->unlock().get();
}
```

fenced_lock works on top of CP sessions. It keeps a CP session open while the lock is acquired. Please refer to the [CP Sessions](https://docs.hazelcast.com/imdg/latest/cp-subsystem/sessions.html) documentation for more information.

Distributed locks are unfortunately *not equivalent* to single-node mutexes because of the complexities in distributed systems, such as uncertain communication patterns, and independent and partial failures. In an asynchronous network, no lock service can guarantee mutual exclusion, because there is no way to distinguish between a slow and a crashed process. Consider the following scenario, where a Hazelcast client acquires a fenced_lock, then hits a long GC pause. Since it will not be able to commit session heartbeats while paused, its CP session will be eventually closed. After this moment, another Hazelcast client can acquire this lock. If the first client wakes up again, it may not immediately notice that it has lost ownership of the lock. In this case, multiple clients think they hold the lock. If they attempt to perform an operation on a shared resource, they can break the system. To prevent such situations, you can choose to use an infinite session timeout, but this time probably you are going to deal with liveliness issues. For the scenario above, even if the first client actually crashes, requests sent by 2 clients can be re-ordered in the network and hit the external resource in reverse order.

There is a simple solution for this problem. Lock holders are ordered by a monotonic fencing token, which increments each time the lock is assigned to a new owner. This fencing token can be passed to external services or resources to ensure sequential execution of the side effects performed by the lock holders.

You can read more about the fencing token idea in Martin Kleppmann's "How to do distributed locking" blog post and Google's Chubby paper.

As an alternative approach, you can use the `try_lock()` method of fenced_lock. It tries to acquire the lock in optimistic manner and immediately returns with either a valid fencing token or `undefined`. It also accepts an optional `timeout` argument which specifies the timeout (in milliseconds resolution) to acquire the lock before giving up.

```c++
// Try to acquire the lock
auto success = lock->try_lock().get();
// Check for valid fencing token
if (success) {
  try {
      // Your guarded code goes here
  } catch (...) {
      // Make sure to release the lock
      lock->unlock().get();
  }
}
```

#### 7.4.11.3. Using counting_semaphore

Hazelcast `counting_semaphore` is the distributed implementation of a linearizable and distributed semaphore. It offers multiple operations for acquiring the permits. This data structure is a part of CP Subsystem.

counting_semaphore is a cluster-wide counting semaphore. Conceptually, it maintains a set of permits. Each `acquire()` waits if necessary until a permit is available, and then takes it. Dually, each `release()` adds a permit, potentially releasing a waiting acquirer. However, no actual permit objects are used; the semaphore just keeps a count of the number available and acts accordingly.

A basic counting_semaphore usage example is shown below.

```c++
// Get counting_semaphore named 'my-semaphore'
auto semaphore = hz.get_cp_subsystem().get_semaphore("my-semaphore").get();
// Try to initialize the semaphore
// (does nothing if the semaphore is already initialized)
semaphore->init(3).get();
// Acquire 3 permits out of 3
semaphore->acquire(3).get();
// Release 2 permits
semaphore->release(2).get();
// Check available permits
auto available = semaphore->available_permits().get();
std::cout << "Available:" << available << "\n";
// Prints:
// Available: 1
```

Beware of the increased risk of indefinite postponement when using the multiple-permit acquire. If permits are released one by one, a caller waiting for one permit will acquire it before a caller waiting for multiple permits regardless of the call order. Correct usage of a semaphore is established by programming convention in the application.

As an alternative, potentially safer approach to the multiple-permit acquire, you can use the `try_acquire()` method of counting_semaphore. It tries to acquire the permits in optimistic manner and immediately returns with a `boolean` operation result. It also accepts an optional `timeout` argument which specifies the timeout (in milliseconds resolution) to acquire the permits before giving up.

```c++
// Try to acquire 1 permit
auto success = semaphore->try_acquire(1).get();
// Check for valid fencing token
if (success) {
    try {
        // Your guarded code goes here
    } catch (...) {
        // Make sure to release the permits
        semaphore->release(1).get();
    }
}
```

 counting_semaphore data structure has two variations:

 * The default implementation is session-aware. In this one, when a caller makes its very first `acquire()` call, it starts a new CP session with the underlying CP group. Then, liveliness of the caller is tracked via this CP session. When the caller fails, permits acquired by this caller are automatically and safely released. However, the session-aware version comes with a limitation, that is, a Hazelcast client cannot release permits before acquiring them first. In other words, a client can release only the permits it has acquired earlier.
 * The second implementation is sessionless. This one does not perform auto-cleanup of acquired permits on failures. Acquired permits are not bound to callers and permits can be released without acquiring first. However, you need to handle failed permit owners on your own. If a Hazelcast server or client fails while holding some permits, they will not be automatically released. You can use the sessionless CP counting_semaphore implementation by enabling JDK compatibility `jdk-compatible` server-side setting. Refer to the [CP Subsystem Configuration](https://docs.hazelcast.com/imdg/latest/cp-subsystem/configuration.html) documentation for more details.

#### 7.4.11.4. Using latch

Hazelcast `latch` is the distributed implementation of a linearizable and distributed countdown latch. This data structure is a cluster-wide synchronization aid that allows one or more callers to wait until a set of operations being performed in other callers completes. This data structure is a part of CP Subsystem.

A basic latch usage example is shown below.

```c++
// Get a latch called 'my-latch'
auto latch = hz.get_cp_subsystem().get_latch("my-latch'").get();

// Try to initialize the latch
// (does nothing if the count is not zero)
auto initialized = latch->try_set_count(1).get();
std::cout << "Initialized:" << initialized << "\n";
// Check count
auto count = latch->get_count().get();
std::cout << "Count:" << count << "\n";
// Prints:
// Count: 1
// Bring the count down to zero after 10ms
auto f = std::async([=] () {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    latch->count_down().get();
});
// Wait up to 1 second for the count to become zero up
auto status = latch->wait_for(std::chrono::seconds(1)).get();
if (status == std::cv_status::no_timeout) {
    std::cout << "latch is count down\n";
}
```

> **NOTE: latch count can be reset with `try_set_count()` after a count_down has finished, but not during an active count.**

#### 7.4.11.5. Using atomic_reference

Hazelcast `atomic_reference` is the distributed implementation of a linearizable object reference. It provides a set of atomic operations allowing to modify the value behind the reference. This data structure is a part of CP Subsystem.

A basic atomic_reference usage example is shown below.

```c++
// Get an atomic_reference named 'my-ref'
auto ref = hz.get_cp_subsystem().get_atomic_reference("my-ref'").get();

// Set the value atomically
ref->set(42).get();
// Read the value
auto value = ref->get<int>().get();
std::cout << "Value:" << value << "\n";
// Prints:
// Value: 42
// Try to replace the value with 'value'
// with a compare-and-set atomic operation
auto result = ref->compare_and_set(42, "value").get();
std::cout << "CAS result:" << result << "\n";
// Prints:
// CAS result: 1
```

The following are some considerations you need to know when you use atomic_reference:

* atomic_reference works based on the byte-content and not on the object-reference. If you use the `compare_and_set()` method, do not change to the original value because its serialized content will then be different.
* All methods returning an object return a private copy. You can modify the private copy, but the rest of the world is shielded from your changes. If you want these changes to be visible to the rest of the world, you need to write the change back to the atomic_reference; but be careful about introducing a data-race.
* The 'in-memory format' of an atomic_reference is `binary`. The receiving side does not need to have the class definition available unless it needs to be deserialized on the other side., e.g., because a method like `alter()` is executed. This deserialization is done for every call that needs to have the object instead of the binary content, so be careful with expensive object graphs that need to be deserialized.
* If you have an object with many fields or an object graph, and you only need to calculate some information or need a subset of fields, you can use the `apply()` method. With the `apply()` method, the whole object does not need to be sent over the line; only the information that is relevant is sent.

atomic_reference does not offer exactly-once / effectively-once execution semantics. It goes with at-least-once execution semantics by default and can cause an API call to be committed multiple times in case of CP member failures. It can be tuned to offer at-most-once execution semantics. Please see the [`fail-on-indeterminate-operation-state`](https://docs.hazelcast.com/imdg/latest/cp-subsystem/configuration.html) server-side setting.

### 7.4.12. Using Transactions

Hazelcast C++ client provides transactional operations like beginning transactions, committing transactions and retrieving transactional data structures like the `transactional_map`, `transactional_set`, `transactional_list`, `transactional_queue` and `transactional_multi_map`.

You can create a `transaction_context` object using the C++ client to begin, commit and rollback a transaction. You can obtain transaction-aware instances of queues, maps, sets, lists and multimaps via the `transaction_context` object, work with them and commit or rollback in one shot. For details, see the [Transactions section](https://docs.hazelcast.com/imdg/latest/transactions/transactions.html) in the Hazelcast IMDG Reference Manual.

```c++
  // Create a Transaction object and begin the transaction
auto context = client->new_transaction_context();
context.begin_transaction().get();

// Get transactional distributed data structures
auto txnMap = context.get_map("transactional-map").get();
auto txnmulti_map = context.get_multi_map("transactional-multimap");
auto txnQueue = context.get_queue("transactional-queue");
auto txnSet = context.get_set("transactional-set");

try {
auto obj = txnQueue->poll<std::string>().get();
//process object
txnMap->put<std::string, std::string>( "1", "value1" ).get();
txnmulti_map->put<std::string, std::string>("2", "value2_1").get();
txnmulti_map->put<std::string, std::string>("2", "value2_2").get();
txnSet->add<std::string>( "value" ).get();
//do other things

// Commit the above changes done in the cluster.
context.commit_transaction().get();
} catch (...) {
context.rollback_transaction().get();
throw;
}
```

In a transaction, operations will not be executed immediately. Their changes will be local to the `transaction_context` until committed. However, they will ensure the changes via locks.

For the above example, when `map->put` is executed, no data will be put in the map but the key will be locked against changes. While committing, operations will be executed, the value will be put to the map and the key will be unlocked.

The isolation level in Hazelcast Transactions is `READ_COMMITTED` on the level of a single partition. If you are in a transaction, you can read the data in your transaction and the data that is already committed. If you are not in a transaction, you can only read the committed data.

## 7.5. Distributed Events

This chapter explains when various events are fired and describes how you can add event listeners on a Hazelcast C++ client. These events can be categorized as cluster and distributed data structure events.

### 7.5.1. Cluster Events

You can add event listeners to a Hazelcast C++ client. You can configure the following listeners to listen to the events on the client side:

* Membership Listener: Notifies when a member joins to/leaves the cluster, or when an attribute is changed in a member.
* Lifecycle Listener: Notifies when the client is starting, started, shutting down and shutdown.

#### 7.5.1.1. Listening for Member Events

You can listen the following types of member events in the `cluster`.

* `member_added`: A new member is added to the cluster.
* `member_removed`: An existing member leaves the cluster.

You can use `cluster` (`hazelcast_client::hazelcast_client::get_cluster()`) object to register for the membership listeners. There are two types of listeners: `InitialMembershipListener` and `membership_listener`. The difference is that `InitialMembershipListener` also gets notified when the client connects to the cluster and retrieves the whole membership list. You need to implement one of these two interfaces and register an instance of the listener to the cluster.

The following example demonstrates both initial and regular membership listener registrations.

```c++
membership_listener make_membership_listener() {
    return membership_listener().on_joined([](const hazelcast::client::membership_event &membership_event) {
      std::cout << "New member joined: "
      << membership_event.get_member().get_address() << std::endl;
    }).on_left([](const hazelcast::client::membership_event &membership_event) {
    std::cout << "Member left: "
    << membership_event.get_member().get_address() << std::endl;
    });
}

membership_listener make_initial_membership_listener() {
    return membership_listener().on_init([](const hazelcast::client::initial_membership_event &event){
      auto members = event.get_members();
      std::cout << "The following are the initial members in the cluster:" << std::endl;
      for (const auto &member : members) {
        std::cout << member.get_address() << std::endl;
      }
    }).on_joined([](const hazelcast::client::membership_event &membership_event) {
      std::cout << "New member joined: " << membership_event.get_member().get_address() << std::endl;
    }).on_left([](const hazelcast::client::membership_event &membership_event) {
      std::cout << "Member left: " <<
      membership_event.get_member().get_address() << std::endl;
    });
}

int main() {
  auto memberListener = make_membership_listener();
  auto initialMemberListener = make_initial_membership_listener();
  
  hazelcast::client::cluster *clusterPtr = nullptr;
  boost::uuids::uuid listenerId, initialListenerId;
  try {
    auto hz = hazelcast::new_client().get();
    
    hazelcast::client::cluster &cluster = hz.get_cluster().get();
    clusterPtr = &cluster;
    auto members = cluster.get_members();
    std::cout << "The following are members in the cluster:" << std::endl;
    for (const auto &member: members) {
        std::cout << member.get_address() << std::endl;
    }
    
    listenerId = cluster.add_membership_listener(std::move(memberListener));
    initialListenerId = cluster.add_membership_listener(std::move(initialMemberListener));
    
    // sleep some time for the events to be delivered before exiting
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    cluster.remove_membership_listener(listenerId).get();
    cluster.remove_membership_listener(initialListenerId).get();
  } catch (hazelcast::client::exception::iexception &e) {
    std::cerr << "failed !!! " << e.what() << std::endl;
    if (nullptr != clusterPtr) {
      clusterPtr->remove_membership_listener(listenerId).get();
      clusterPtr->remove_membership_listener(initialListenerId).get();
    }
    exit(-1);
  }
...
```

#### 7.5.1.3. Listening for Lifecycle Events

The `lifecycle_listener` interface notifies for the following events:

* `starting`: The client is starting.
* `started`: The client has started.
* `shutting_down`: The client is shutting down.
* `shutdown`: The client’s shutdown has completed.
* `client_connected`: The client is connected to the cluster.
* `client_disconnected`: The client is disconnected from the cluster.

The following is an example of the `lifecycle_listener` that is added to the `client_config` object and its output.

```c++
int main() {
    hazelcast::client::client_config config;
    
    // Add a lifecycle listener so that we can track when the client is connected/disconnected
    config.add_listener(
    hazelcast::client::lifecycle_listener().on_connected([](){
        std::cout << "Client connected to the cluster" << std::endl;
      }).on_disconnected([] () {
          std::cout << "Client is disconnected from the cluster" << std::endl;
      });
    );
    
    auto hz = hazelcast::new_client(std::move(config)).get();
    
    hz.shutdown().get();
    return 0;
  }
```

**Output:**

```
20/11/2020 12:26:43.340 INFO: [0x11a0acdc0] hz.client_1[dev] [5.0.0] [../hazelcast/src/hazelcast/client/spi.cpp:375] (Fri Nov 20 11:03:59 2020 +0300:f2326084c3) LifecycleService::LifecycleEvent Client (7add62a3-c6ec-4002-a9d8-79ed4639f8e9) is STARTING
20/11/2020 12:26:43.343 INFO: [0x11a0acdc0] hz.client_1[dev] [5.0.0] [../hazelcast/src/hazelcast/client/spi.cpp:379] (Fri Nov 20 11:03:59 2020 +0300:f2326084c3) LifecycleService::LifecycleEvent STARTING
20/11/2020 12:26:43.343 INFO: [0x11a0acdc0] hz.client_1[dev] [5.0.0] [../hazelcast/src/hazelcast/client/spi.cpp:387] LifecycleService::LifecycleEvent STARTED
20/11/2020 12:26:43.398 INFO: [0x11a0acdc0] hz.client_1[dev] [5.0.0] [../hazelcast/src/hazelcast/client/network.cpp:587] Trying to connect to Address[127.0.0.1:5701]
20/11/2020 12:26:43.409 INFO: [0x11a0acdc0] hz.client_1[dev] [5.0.0] [../hazelcast/src/hazelcast/client/spi.cpp:411] LifecycleService::LifecycleEvent CLIENT_CONNECTED
20/11/2020 12:26:43.410 INFO: [0x11a0acdc0] hz.client_1[dev] [5.0.0] [../hazelcast/src/hazelcast/client/network.cpp:637] Authenticated with server  Address[:5701]:8324bc53-8190-400c-bcbf-e582834a0542, server version: 4.2, local address: Address[127.0.0.1:63383]
20/11/2020 12:26:43.412 INFO: [0x7000062b8000] hz.client_1[dev] [5.0.0] [../hazelcast/src/hazelcast/client/spi.cpp:881] 

Members [1]  {
	Member[localhost]:5701 - 8324bc53-8190-400c-bcbf-e582834a0542
}
Client connected to the cluster
20/11/2020 12:26:48.700 INFO: [0x11a0acdc0] hz.client_1[dev] [5.0.0] [../hazelcast/src/hazelcast/client/spi.cpp:395] LifecycleService::LifecycleEvent SHUTTING_DOWN
20/11/2020 12:26:48.701 INFO: [0x11a0acdc0] hz.client_1[dev] [5.0.0] [../hazelcast/src/hazelcast/client/network.cpp:545] Removed connection to endpoint: Address[localhost:5701], connection: ClientConnection{alive=0, connectionId=1, remoteEndpoint=Address[localhost:5701], lastReadTime=2020-11-20 12:26:43.-286, closedTime=2020-11-20 12:26:48.000, connected server version=4.2}
20/11/2020 12:26:48.701 INFO: [0x11a0acdc0] hz.client_1[dev] [5.0.0] [../hazelcast/src/hazelcast/client/spi.cpp:419] LifecycleService::LifecycleEvent CLIENT_DISCONNECTED
Client is disconnected from the cluster
20/11/2020 12:26:48.703 INFO: [0x11a0acdc0] hz.client_1[dev] [5.0.0] [../hazelcast/src/hazelcast/client/spi.cpp:403] LifecycleService::LifecycleEvent SHUTDOWN
```

### 7.5.2. Distributed Data Structure Events

You can add event listeners to the distributed data structures.

#### 7.5.2.1. Listening for Map Events

You can listen to map-wide or entry-based events by registering an instance of the `entry_listener` class. You should provide the `entry_listener` instance with function objects for each type of event you want to listen to and then use `imap::add_entry_listener` to register it.

An entry-based event is fired after operations that affect a specific entry. For example, `imap::put()`, `imap::remove()` or `imap::evict()`. The corresponding function that you provided to the listener will be called with an `entry_event` when an entry-based event occurs.

See the following example.

```c++
int main() {
  auto hz = hazelcast::new_client().get();
  
  auto map = hz.get_map("EntryListenerExampleMap").get();
  
  auto registrationId = map->add_entry_listener(
  entry_listener().on_added([](hazelcast::client::EntryEvent &&event) {
        std::cout << "Entry added:" << event.getKey().get<int>().value() << " --> " << event.getValue().get<std::string>().value() << std::endl;
    }).on_removed([](hazelcast::client::EntryEvent &&event) {
        std::cout << "Entry removed:" << event.getKey().get<int>().value()
        << " --> " << event.getValue().get<std::string>().value() << std::endl;
    }), true).get();
  
  map->put(1, "My new entry").get();
  map->remove<int, std::string>(1).get();
  
  /* ... */
  
  return 0;
}
```

A map-wide event is fired as a result of a map-wide operation (e.g.: `imap::clear()`, `imap::evict_all()`) which can also be listened to via an `entry_listener`. The functions you provided to the listener will be called with a `map_event` when a map-wide event occurs.

See the example below.

```c++
int main() {
  auto hz = hazelcast::new_client().get();
  
  auto map = hz.get_map("EntryListenerExampleMap").get();
  
  auto registrationId = map->add_entry_listener(
  entry_listener().on_map_cleared([](hazelcast::client::map_event &&event) {
          std::cout << "Map cleared:" << event.getNumberOfEntriesAffected() << std::endl; // Map Cleared: 3
      }), false).get();
  
  map->put(1, "Mali").get();
  map->put(2, "Ahmet").get();
  map->put(3, "Furkan").get();
  map->clear().get();
  
  /* ... */
  
  return 0;
}
```

## 7.6. Distributed Computing

This section describes how Hazelcast IMDG's distributed executor service and entry processor features can be used in the C++ client.

### 7.6.1. Distributed Executor Service
Hazelcast C++ client allows you to asynchronously execute your tasks (logical units of work) in the cluster, such as database queries, complex calculations and image rendering.

With `iexecutor_service`, you can execute tasks asynchronously and perform other useful tasks. If your task execution takes longer than expected, you can cancel the task execution. Tasks should be Hazelcast serializable, since they will be distributed in the cluster.

You need to implement the actual task logic at the server side as a Java code. The task should implement Java's `java.util.concurrent.Callable` interface.

Note that, the distributed executor service (`iexecutor_service`) is intended to run processing where the data is hosted: on the server members.

For more information on the server side configuration, see the [Executor Service section](https://docs.hazelcast.com/imdg/latest/computing/executor-service.html) in the Hazelcast IMDG Reference Manual.

#### 7.6.1.1 Implementing a Callable Task

You implement a C++ class which is Hazelcast serializable. This is the task class to be run on the server side. The client side implementation does not need to have any logic, it is purely for initiating the server side task.

On the server side, when you implement the task as `java.util.concurrent.Callable` (a task that returns a value), implement one of the Hazelcast serialization methods for the new class. The serialization type needs to match with that of the client side task class.

An example C++ task class implementation is shown below.

```c++
struct MessagePrinter {
std::string message;
};

namespace hazelcast {
  namespace client {
    namespace serialization {
      template<>
        struct hz_serializer<MessagePrinter> : identified_data_serializer {
        static int32_t get_factory_id() noexcept {
            return 1;
        }
      
        static int32_t get_class_id() noexcept {
        return 555;
        }
      
        static void write_data(const MessagePrinter &object, object_data_output &out) {
          out.write(object.message);
        }
      
        static MessagePrinter read_data(object_data_input &in) {
            return MessagePrinter{in.read<std::string>(); }
        }
      };
    }
  }
}
```

An example of a Callable Java task which matches the above C++ class is shown below. `MessagePrinter` prints out the message sent from the C++ client at the cluster members.

```Java
// The Java server side example MessagePrinter implementation looks like the following (Please observe that Java class
// should implement the Callable and the IdentifiedDataSerializable interfaces):
//
public class MessagePrinter implements IdentifiedDataSerializable, Callable<String> {
    private String message;

    public MessagePrinter(String message) {
        this.message = message;
    }

    @Override
    public int getFactoryId() {
        return 1;
    }

    @Override
    public int getId() {
        return 555;
    }

    @Override
    public void writeData(object_data_output out)
            throws io {
        out.writeUTF(message);
    }

    @Override
    public void readData(object_data_input in)
            throws io {
        message = in.readUTF();
    }

    @Override
    public String call() throws Exception {
        System.out.println(message);
        return message;
    }
}
```

You need to compile and link the Java class on the server side (add it to the server's classpath), implement and register a DataSerializableFactory on the server side. In this example, we designed the task as `IdentifiedDataSerializable` and you can see that the factory and class ID for both C++ and Java classes are the same. You can of course use the other Hazelcast serialization methods as well.

#### 7.6.1.2 Executing a Callable Task

To execute a callable task:

* Retrieve the executor from `hazelcast_client`.
* Submit a task which returns a `future`.
* After executing the task, you do not have to wait for the execution to complete, you can process other things.
* When ready, use the `future` object to retrieve the result as shown in the code example below.

An example where `MessagePrinter` task is executed is shown below.

```c++
// Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
auto hz = hazelcast::new_client().get();
// Get the Distributed Executor Service
std::shared_ptr<iexecutor_service> ex = hz.get_executor_service("my-distributed-executor").get();
// Submit the MessagePrinter Runnable to a random Hazelcast Cluster Member
auto result_future = ex->submit<MessagePrinter, std::string>(MessagePrinter{ "message to any node" });
// Wait for the result of the submitted task and print the result
auto result = result_future.get_future().get();
std::cout << "Server result: " << *result << std::endl;
```

#### 7.6.1.3 Scaling The Executor Service

You can scale the Executor service both vertically (scale up) and horizontally (scale out). See the [Scaling The Executor Service section](https://docs.hazelcast.com/imdg/latest/computing/executor-service.html#scaling-the-executor-service) in the Hazelcast IMDG Reference Manual for more details on its configuration.

#### 7.6.1.4 Executing Code in the Cluster

The distributed executor service allows you to execute your code in the cluster. In this section, the code examples are based on the `MessagePrinter` class above. The code examples show how Hazelcast can execute your code:

* `printOnTheMember`: On a specific cluster member you choose with the `iexecutor_service::submit_to_member` method.
* `printOnTheMemberOwningTheKey`: On the member owning the key you choose with the `iexecutor_service::submit_to_key_owner` method.
* `printOnSomewhere`: On the member Hazelcast picks with the `iexecutor_service::submit` method.
* `printOnMembers`: On all or a subset of the cluster members with the `iexecutor_service::submit_to_members` method.

```c++
void printOnTheMember(const std::string &input, const Member &member) {
// Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
auto hz = hazelcast::new_client().get();
// Get the Distributed Executor Service
std::shared_ptr<iexecutor_service> ex = hz.get_executor_service("my-distributed-executor").get();
// Get the first Hazelcast Cluster Member
member firstMember = hz.get_cluster().get_members()[0];
// Submit the MessagePrinter Runnable to the first Hazelcast Cluster Member
ex->execute_on_member<MessagePrinter>(MessagePrinter{ "message to very first member of the cluster" }, firstMember).get();
```

```c++
void printOnTheMemberOwningTheKey(const std::string &input, const std::string &key) {
  // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
  auto hz = hazelcast::new_client().get();
  // Get the Distributed Executor Service
  std::shared_ptr<iexecutor_service> ex = hz.get_executor_service("my-distributed-executor").get();
  // Submit the MessagePrinter Runnable to the Hazelcast Cluster Member owning the key called "key"
  ex->execute_on_key_owner<MessagePrinter, std::string>(MessagePrinter{"message to the member that owns the key"}, "key").get();
}
```

```c++
void printOnSomewhere(const std::string &input) {
  // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
  auto hz = hazelcast::new_client().get();
  // Get the Distributed Executor Service
  std::shared_ptr<iexecutor_service> ex = hz.get_executor_service("my-distributed-executor").get();
  // Submit the MessagePrinter Runnable to a random Hazelcast Cluster Member
  auto result_future = ex->submit<MessagePrinter, std::string>(MessagePrinter{ "message to any node" }).get();
}
```

```c++
void printOnMembers(const std::string input, const std::vector<Member> &members) {
  // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
  auto hz = hazelcast::new_client().get();
  // Get the Distributed Executor Service
  std::shared_ptr<iexecutor_service> ex = hz.get_executor_service("my-distributed-executor").get();
  ex->execute_on_members<MessagePrinter>(MessagePrinter{ "message to very first member of the cluster" }, members);
}
```

**NOTE:** You can obtain the list of cluster members via the `hazelcast_client::get_cluster()::get_bembers()` call.

#### 7.6.1.5 Canceling an Executing Task
A task in the code that you execute in a cluster might take longer than expected. If you cannot stop/cancel that task, it will keep eating your resources.

To cancel a task, you can use the `executor_promise<T>::cancel()` API. This API lets you to code and design for cancellations, a highly ignored part of software development. Please keep in mind that if the execution is already started or finished, the API may not be able to cancel the task. 

#### 7.6.1.5.1 Example Task to Cancel
The following code waits for the task to be completed in 3 seconds. If it is not finished within this period, a `timeout` is thrown from the `get()` method, and we cancel the task with the `cancel()` method. The remote execution of the task is being cancelled.

```
auto service = client->get_executor_service(get_test_name()).get();

executor::tasks::CancellationAwareTask task{INT64_MAX};

auto promise = service->submit<executor::tasks::CancellationAwareTask, bool>(task);

auto future = promise.get_future();
if (future.wait_for(boost::chrono::seconds(3) == boost::future_status::timeout) {
    std::cout << "Could not get response back in 3 seconds\n";
}

if (promise.cancel(true)) {
    std::cout << "Cancelled the submitted task\n";
}
```

#### 7.6.1.6 Selecting Members for Task Execution
As previously mentioned, it is possible to indicate where in the Hazelcast cluster the task is executed. Usually you execute these in the cluster based on the location of a key or set of keys, or you allow Hazelcast to select a member.

If you want more control over where your code runs, use the `member_selector` interface. For example, you may want certain tasks to run only on certain members, or you may wish to implement some form of custom load balancing regime.  The `member_selector` is an interface that you can implement and then provide to the `iexecutor_service` when you submit or execute.

The `bool select(const Member &member)` method is called for every available member in the cluster. Implement this method to decide if the member is going to be used or not.

In the simple example shown below, we select the cluster members based on the presence of an attribute.

```c++
    class MyMemberSelector :
public member_selector {
public:
bool select(const member &member) const override {
const std::string *attribute = member.get_attribute("my.special.executor");
if (attribute == NULL) {
return false;
}

return *attribute == "true";
}
};
```

You can now submit your task using this selector and the task will run on the member whose attribute key "my.special.executor" is set to "true". An example is shown below:

```
// Choose which member to submit the task to using a member selector
ex->submit<MessagePrinter, std::string>(MessagePrinter{"Message when using the member selector"},
                                       MyMemberSelector());
```

### 7.6.2. Using entry_processor

Hazelcast supports entry processing. An entry processor is a function that executes your code on a map entry in an atomic way.

An entry processor is a good option if you perform bulk processing on an `imap`. Usually you perform a loop of keys -- executing `imap->get(key)`, mutating the value and finally putting the entry back in the map using `imap->put(key,value)`. If you perform this process from a client or from a member where the keys do not exist, you effectively perform two network hops for each update: the first to retrieve the data and the second to update the mutated value.

If you are doing the process described above, you should consider using entry processors. An entry processor executes a read and updates upon the member where the data resides. This eliminates the costly network hops described above.

> **NOTE: Entry processor is meant to process a single entry per call. Processing multiple entries and data structures in an entry processor is not supported as it may result in deadlocks on the server side.**

Hazelcast sends the entry processor to each cluster member and these members apply it to the map entries. Therefore, if you add more members, your processing completes faster.

#### 7.6.2.1. Processing Entries

The `imap` interface provides the following functions for entry processing:

* `execute_on_key` processes an entry mapped by a key.
* `execute_on_keys` processes entries mapped by a list of keys.
* `execute_on_entries` can process all entries in a map with a defined predicate. predicate is optional.

In the C++ client, an `entry_processor` should be Hazelcast serializable, because the server should be able to deserialize it to process.

The following is an example for `entry_processor` which is serialized by `identified_data_serializer`.

```c++
struct IdentifiedEntryProcessor {
    std::string name;
};

namespace hazelcast {
    namespace client {
      namespace serialization {
        template<>
        struct hz_serializer<IdentifiedEntryProcessor> : identified_data_serializer {
          static int32_t get_factory_id() noexcept {
              return 66;
          }
  
          static int32_t get_class_id() noexcept {
              return 1;
          }
          
          static void
          write_data(const IdentifiedEntryProcessor &object, object_data_output &out) {
              out.write(object.name);
          }
          
          static IdentifiedEntryProcessor read_data(object_data_input &in) {
            // no-need to implement here, since we do not expect to receive this object but we only send it to server
            assert(0);
            return IdentifiedEntryProcessor{};
          }
        };
      }
    }
}
```

Now, you need to make sure that the Hazelcast member recognizes the entry processor. For this, you need to implement the Java equivalent of your entry processor and its factory, and create your own compiled class or JAR files. For adding your own compiled class or JAR files to the server's `CLASSPATH`, see the [Adding User Library to CLASSPATH section](#1214-adding-user-java-library-to-java-classpath).

The following is the Java equivalent of the entry processor in C++ client given above:

```java
public class IdentifiedEntryProcessor implements EntryProcessor<String, String, String>, IdentifiedDataSerializable {

    static final int CLASS_ID = 1;

    private String value;

    public IdentifiedEntryProcessor() {
    }

    @Override
    public int getFactoryId() {
        return IdentifiedFactory.FACTORY_ID;
    }

    @Override
    public int getClassId() {
        return CLASS_ID;
    }

    @Override
    public void writeData(ObjectDataOutput out) throws IOException {
        out.writeUTF(value);
    }

    @Override
    public void readData(ObjectDataInput in) throws IOException {
        value = in.readUTF();
    }

    @Override
    public String process(Map.Entry<String, String> entry) {
        entry.setValue(value);
        return value;
    }
}
```

You can implement the above processor’s factory as follows:

```java
public class IdentifiedFactory implements DataSerializableFactory {
    public static final int FACTORY_ID = 66;

    @Override
    public IdentifiedDataSerializable create(int typeId) {
        if (typeId == IdentifiedEntryProcessor.CLASS_ID) {
            return new IdentifiedEntryProcessor();
        }
        ...
        return null;
    }   
```

Now you need to configure the `hazelcast.xml` to add your factory as shown below.

```xml
<hazelcast>
    <serialization>
        <data-serializable-factories>
            <data-serializable-factory factory-id="66">
                com.hazelcast.client.test.IdentifiedFactory
            </data-serializable-factory>
        </data-serializable-factories>
    </serialization>
</hazelcast>
```

The code that runs on the entries is implemented in Java on the server side. The client side entry processor is used to specify which entry processor should be called. For more details about the Java implementation of the entry processor, see the [Entry Processor section](https://docs.hazelcast.com/imdg/latest/computing/entry-processor.html) in the Hazelcast IMDG Reference Manual.

After the above implementations and configuration are done and you start the server where your library is added to its `CLASSPATH`, you can use the entry processor in the `imap` functions. See the following example.

```c++
auto map = hz.get_map("my-distributed-map").get();
map->execute_on_key<std::string, IdentifiedEntryProcessor>("key", IdentifiedEntryProcessor("my new name"));
std::cout << "Value for 'key' is : '" << *map->get<std::string, std::string>("key").get() << "'" << std::endl; // value for 'key' is 'my new name'
```

## 7.7. Distributed Query

Hazelcast partitions your data and spreads it across cluster of members. You can iterate over the map entries and look for certain entries (specified by predicates) you are interested in. However, this is not very efficient because you will have to bring the entire entry set and iterate locally. Instead, Hazelcast allows you to run distributed queries on your distributed map.

### 7.7.1. How Distributed Query Works

1. The requested predicate is sent to each member in the cluster.
2. Each member looks at its own local entries and filters them according to the predicate. At this stage, key-value pairs of the entries are deserialized and then passed to the predicate.
3. The predicate requester merges all the results coming from each member into a single set.

Distributed query is highly scalable. If you add new members to the cluster, the partition count for each member is reduced and thus the time spent by each member on iterating its entries is reduced. In addition, the pool of partition threads evaluates the entries concurrently in each member, and the network traffic is also reduced since only filtered data is sent to the requester.

If queried item is Portable, it can be queried for the fields without deserializing the data at the server side and hence no server side implementation of the queried object class will be needed. 

**Built-in Predicates For Query**

There are many built-in `predicate` implementations for your query requirements. Some of them are explained below.

* `true_predicate`: This predicate returns true and hence includes all the entries on the response.
* `false_predicate`: This predicate returns false and hence filters out all the entries in the response.
* `equal_predicate`: Checks if the result of an expression is equal to a given value.
* `not_equal_predicate`: Checks if the result of an expression is not equal to a given value.
* `instance_of_predicate`: Checks if the result of an expression has a certain type.
* `like_predicate`: Checks if the result of an expression matches some string pattern. `%` (percentage sign) is the placeholder for many characters, `_` (underscore) is placeholder for only one character.
* `ilike_predicate`: Same as like_predicate. Checks if the result of an expression matches some string pattern. `%` (percentage sign) is the placeholder for many characters, `_` (underscore) is placeholder for only one character.
* `greater_less_predicate`: Checks if the result of an expression is greater equal or less equal than a certain value.
* `between_predicate`: Checks if the result of an expression is between two values (start and end values are inclusive).
* `in_predicate`: Checks if the result of an expression is an element of a certain list.
* `not_predicate`: Negates a provided predicate result.
* `regex_predicate`: Checks if the result of an expression matches some regular expression.
* `sql_predicate`: Query using SQL syntax.
* `paging_predicate`: Returns matching entries page by page.

Hazelcast offers the following ways for distributed query purposes:

* Combining predicates with `and_predicate`, `or_predicate` and `not_predicate`
* Distributed SQL Query

#### 7.7.1.1. person Map Query Example

Assume that you have a `person` map containing the values of `person` objects, as coded below. 

```c++
struct person {
    std::string name;
    bool male;
    int32_t age;
};

std::ostream &operator<<(std::ostream &os, const person &obj) {
  os << "name: " << obj.name << " male: " << obj.male << " age: " << obj.age;
  return os;
}

namespace hazelcast {
    namespace client {
        namespace serialization {
          template<>
          struct hz_serializer<person> : portable_serializer {
            static int32_t get_factory_id() noexcept {
              return 1;
            }
            
            static int32_t get_class_id() noexcept {
              return 3;
            }
            
            static void write_data(const person &object, portable_writer &out) {
              out.write(object.name);
              out.write(object.male);
              out.write(object.age);
            }
            
            static person read_data(portable_reader &in) {
              return person{in.read<std::string>(), in.read<bool>(), in.read<int32_t>()};
            }
          };
        }
    }
}
```

Note that `person` is being serialized with portable_serializer`. As portable types are not deserialized on the server side for querying, you don't need to implement its Java equivalent on the server side for querying.

For the non-portable types, you need to implement its Java equivalent and its serializable factory on the server side for server to reconstitute the objects from binary formats. 
In this case before starting the server, you need to compile the `person` and related factory classes with server's `CLASSPATH` and add them to the `user-lib` directory in the extracted `hazelcast-<version>.zip` (or `tar`). See the [Adding User Library to CLASSPATH section](#1214-adding-user-java-library-to-java-classpath).

> **NOTE: Querying with `portable_serializer` is faster as compared to `identified_data_serializer` .**

#### 7.7.1.2. Querying by Combining Predicates with AND, OR, NOT

You can combine predicates by using the `and_predicate`, `or_predicate` and `not_predicate` operators, as shown in the below example.

```c++
// and_predicate
// 5 <= key <= 10 AND Values in {4, 10, 19} = values {4, 10}
std::vector<int> inVals{4, 10, 19};
values = intMap->values<int>(query::and_predicate(client,
                                                  query::between_predicate(client,
                                                                          query::query_constants::KEY_ATTRIBUTE_NAME,
                                                                          5, 10),
                                                  query::in_predicate(client,
                                                                     query::query_constants::THIS_ATTRIBUTE_NAME,
                                                                     inVals))).get();

// 5 <= key <= 10 OR Values in {4, 10, 19} = values {4, 10, 12, 14, 16, 18, 20}
values = intMap->values<int>(query::or_predicate(client,
                                                 query::between_predicate(client,
                                                                         query::query_constants::KEY_ATTRIBUTE_NAME,
                                                                         5, 10),
                                                 query::in_predicate(client,
                                                                    query::query_constants::THIS_ATTRIBUTE_NAME,
                                                                    inVals))).get();

// not_predicate
// !(5 <= key <= 10)
values = intMap->values<int>(query::not_predicate(client, query::between_predicate(client,
                                                                                   query::query_constants::KEY_ATTRIBUTE_NAME,
                                                                                   5, 10))).get();
```
See examples/distributed-map/query folder for more examples.

#### 7.7.1.3. Querying with SQL

`sql_predicate` takes the regular SQL `where` clause. See the following example:

```c++
// sql_predicate
// __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
auto sql = (boost::format("%1% BETWEEN 4 and 7") % query::query_constants::KEY_ATTRIBUTE_NAME).str();
values = intMap->values<int>(query::sql_predicate(client, sql)).get();
```

##### 7.7.1.3.1. Supported SQL Syntax

**AND/OR:** `<expression> AND <expression> AND <expression>…`
   
- `active AND age > 30`
- `active = false OR age = 45 OR name = 'Joe'`
- `active AND ( age > 20 OR salary < 60000 )`

**Equality:** `=, !=, <, ⇐, >, >=`

- `<expression> = value`
- `age <= 30`
- `name = 'Joe'`
- `salary != 50000`

**BETWEEN:** `<attribute> [NOT] BETWEEN <value1> AND <value2>`

- `age BETWEEN 20 AND 33 ( same as age >= 20 AND age ⇐ 33 )`
- `age NOT BETWEEN 30 AND 40 ( same as age < 30 OR age > 40 )`

**IN:** `<attribute> [NOT] IN (val1, val2,…)`

- `age IN ( 20, 30, 40 )`
- `age NOT IN ( 60, 70 )`
- `active AND ( salary >= 50000 OR ( age NOT BETWEEN 20 AND 30 ) )`
- `age IN ( 20, 30, 40 ) AND salary BETWEEN ( 50000, 80000 )`

**LIKE:** `<attribute> [NOT] LIKE 'expression'`

The `%` (percentage sign) is the placeholder for multiple characters, an `_` (underscore) is the placeholder for only one character.

- `name LIKE 'Jo%'` (true for 'Joe', 'Josh', 'Joseph' etc.)
- `name LIKE 'Jo_'` (true for 'Joe'; false for 'Josh')
- `name NOT LIKE 'Jo_'` (true for 'Josh'; false for 'Joe')
- `name LIKE 'J_s%'` (true for 'Josh', 'Joseph'; false 'John', 'Joe')

**ILIKE:** `<attribute> [NOT] ILIKE 'expression'`

ILIKE is similar to the LIKE predicate but in a case-insensitive manner.

- `name ILIKE 'Jo%'` (true for 'Joe', 'joe', 'jOe','Josh','joSH', etc.)
- `name ILIKE 'Jo_'` (true for 'Joe' or 'jOE'; false for 'Josh')

**REGEX:** `<attribute> [NOT] REGEX 'expression'`

- `name REGEX 'abc-.*'` (true for 'abc-123'; false for 'abx-123')

#### 7.7.1.4. Querying Examples with Predicates

You can use the `query::query_constants::KEY_ATTRIBUTE_NAME` (`__key`) attribute to perform a predicated search for entry keys. See the following example:

```c++
auto map = hz.get_map("personMap").get();
map->put<std::string, int>("Mali", 28).get();
map->put<std::string, int>("Ahmet", 30).get();
map->put<std::string, int>("Furkan", 23).get();
query::sql_predicate sqlPredicate("__key like F%");
auto startingWithF = map->entry_set<std::string, int>(sqlPredicate).get();
std::cout << "First person:" << startingWithF[0].first << ", age:" << startingWithF[0].second << std::endl;
```

In this example, the code creates a list with the values whose keys start with the letter "F”.

You can use `query::query_constants::THIS_ATTRIBUTE_NAME` (`this`) attribute to perform a predicated search for entry values. See the following example:

```c++
auto map = hz.get_map("personMap").get();
map->put<std::string, int>("Mali", 28).get();
map->put<std::string, int>("Ahmet", 30).get();
map->put<std::string, int>("Furkan", 23).get();
query::greater_less_predicate<int> greaterThan27Predicate(query::query_constants::THIS_ATTRIBUTE_NAME, 27, false, false);
std::vector<std::string> olderThan27 = map->keySet<std::string>(greaterThan27Predicate).get();
std::cout << "First person:" << olderThan27[0] << ", second person:" << olderThan27[1] << std::endl;
```

In this example, the code creates a list with the values greater than or equal to "27".

#### 7.7.1.5. Querying with JSON Strings

You can query JSON strings stored inside your Hazelcast clusters. To query the JSON string,
you first need to create a `hazelcast_json_value` from the JSON string using the `hazelcast_json_value(const std::string &)`
constructor. You can use ``hazelcast_json_value``s both as keys and values in the distributed data structures. Then, it is
possible to query these objects using the Hazelcast query methods explained in this section.

```c++
std::string person1 = "{ \"name\": \"John\", \"age\": 35 }";
std::string person2 = "{ \"name\": \"Jane\", \"age\": 24 }";
std::string person3 = "{ \"name\": \"Trey\", \"age\": 17 }";

auto idPersonMap = hz.get_map("jsonValues").get();

idPersonMap->put<int, hazelcast_json_value>(1, hazelcast_json_value(person1)).get();
idPersonMap->put<int, hazelcast_json_value>(2, hazelcast_json_value(person2)).get();
idPersonMap->put<int, hazelcast_json_value>(3, hazelcast_json_value(person3)).get();

auto peopleUnder21 = idPersonMap.values(query::greater_less_predicate<int>("age", 21, false, true)).get();
```

When running the queries, Hazelcast treats values extracted from the JSON documents as Java types so they
can be compared with the query attribute. JSON specification defines five primitive types to be used in the JSON
documents: `number`,`string`, `true`, `false` and `null`. The `string`, `true/false` and `null` types are treated
as `String`, `boolean` and `null`, respectively. We treat the extracted `number` values as ``long``s if they
can be represented by a `long`. Otherwise, ``number``s are treated as ``double``s.

It is possible to query nested attributes and arrays in the JSON documents. The query syntax is the same
as querying other Hazelcast objects using the ``predicate``s.

```c++
/**
 * Sample JSON object
 *
 * {
 *     "departmentId": 1,
 *     "room": "alpha",
 *     "people": [
 *         {
 *             "name": "Peter",
 *             "age": 26,
 *             "salary": 50000
 *         },
 *         {
 *             "name": "Jonah",
 *             "age": 50,
 *             "salary": 140000
 *         }
 *     ]
 * }
 *
 *
 * The following query finds all the departments that have a person named "Peter" working in them.
 */
auto departmentWithPeter = departments->values(query::equal_predicate<std::string>("people[any].name", "Peter")).get();
```

`hazelcast_json_value` is a lightweight wrapper around your JSON strings. It is used merely as a way to indicate
that the contained string should be treated as a valid JSON value. Hazelcast does not check the validity of JSON
strings put into to the maps. Putting an invalid JSON string into a map is permissible. However, in that case
whether such an entry is going to be returned or not from a query is not defined.

#### 7.7.1.6. Filtering with Paging Predicates

The C++ client provides paging for defined predicates. With its `paging_predicate` object, you can get a list of keys, values, or entries page by page by filtering them with predicates and giving the size of the pages. Also, you can sort the entries by specifying comparators.

```c++
auto map = hz.get_map("personMap").get();
// paging_predicate with inner predicate (value < 10)
auto predicate = intMap->new_paging_predicate<int, int>(5,
        query::greater_less_predicate(client, query::query_constants::THIS_ATTRIBUTE_NAME, 10, false, false));

// Set page to retrieve third page
pagingPredicate.set_page(3);
auto values = map->values(pagingPredicate).get();
//...

// Set up next page
pagingPredicate.next_page();

// Retrieve next page    
values = map->values(pagingPredicate).get();

//...
```

If you want to sort the result before paging, you need to specify a comparator object that implements the `query::entry_comparator` interface. Also, this comparator object should be Hazelcast serializable. After implementing After implementing this object in C++, you need to implement the Java equivalent of it and its factory. The Java equivalent of the comparator should implement `java.util.Comparator`. Note that the `compare` function of `Comparator` on the Java side is the equivalent of the `sort` function of `Comparator` on the C++ side. When you implement the `Comparator` and its factory, you can add them to the `CLASSPATH` of the server side.  See the [Adding User Library to CLASSPATH section](#1214-adding-user-java-library-to-java-classpath). 

Also, you can access a specific page more easily with the help of the `set_page` function. This way, if you make a query for the 100th page, for example, it will get all 100 pages at once instead of reaching the 100th page one by one using the `next_page` function.

## 7.8. Performance

### 7.8.1. Partition Aware

Partition Aware ensures that the related entries exist on the same member. If the related data is on the same member, operations can be executed without the cost of extra network calls and extra wire data, and this improves the performance. This feature is provided by using the same partition keys for related data.

Hazelcast has a standard way of finding out which member owns/manages each key object. The following operations are routed to the same member, since all of them are operating based on the same key `'key1'`.

```c++
hazelcast::client::hazelcast_client hazelcastInstance;

auto mapA = hazelcastInstance.get_map("mapA");
auto mapB = hazelcastInstance.get_map("mapB");
auto mapC = hazelcastInstance.get_map("mapC");

// since map names are different, operation will be manipulating
// different entries, but the operation will take place on the
// same member since the keys ("key1") are the same
mapA->put("key1", value);
mapB.get("key1");
mapC.remove("key1");
```

When the keys are the same, entries are stored on the same member. However, we sometimes want to have the related entries stored on the same member, such as a customer and his/her order entries. We would have a customers map with `customerId` as the key and an orders map with `orderId` as the key. Since `customerId` and `orderId` are different keys, a customer and his/her orders may fall into different members in your cluster. So how can we have them stored on the same member? We create an affinity between the customer and orders. If we make them part of the same partition then these entries will be co-located. We achieve this by making `orderKey` s `partition_aware`.

```c++
struct OrderKey : public hazelcast::client::partition_aware<std::string> {
    OrderKey(const std::string &order_id_string) : order_id_string(order_id_string) {}

    const std::string *get_partition_key() const override {
        return &customer_id_string;
    }

    std::string order_id_string;
    static const std::string customer_id_string;
};
const std::string OrderKey::customer_id_string = "My customer id 99";

namespace hazelcast {
    namespace client {
        namespace serialization {
          template<>
          struct hz_serializer<OrderKey> : identified_data_serializer {
            static int32_t get_factory_id() noexcept {
              return 1;
            }
            
            static int32_t get_class_id() noexcept {
              return 10;
            }
            
            static void write_data(const OrderKey &object, object_data_output &out) {
              out.write(object.order_id_string);
            }
            
            static OrderKey read_data(object_data_input &in) {
              return OrderKey{in.read<std::string>()};
            }
          };
        }
    }
}
```

Notice that `OrderKey` implements `partition_aware` interface and that `get_partition_key()` returns the `OrderKey::customer_id_string`. This will make sure that the `Customer` entry and its `Order`s will be stored on the same member.

```c++
hazelcast::client::hazelcast_client hazelcastInstance;

auto mapCustomers = hazelcastInstance.get_map("customers");
auto mapOrders = hazelcastInstance.get_map("orders");

// create the customer entry with customer id = "My customer id 99"
mapCustomers->put(OrderKey::customer_id_string, customer).get();

// now create the orders for this customer
mapOrders->put<OrderKey, Order>(OrderKey{"1"}, order1).get();
mapOrders->put<OrderKey, Order>(OrderKey{"2"}, order2).get();
mapOrders->put<OrderKey, Order>(OrderKey{"3"}, order3).get();
```  

For more details, see the [Data Affinity section](https://docs.hazelcast.com/imdg/latest/performance/data-affinity.html) in the Hazelcast IMDG Reference Manual.

### 7.8.2. Near Cache

Map  entries in Hazelcast are partitioned across the cluster members. Hazelcast clients do not have local data at all. Suppose you read the key `k` a number of times from a Hazelcast client or `k` is owned by another member in your cluster. Then each `map->get(k)` will be a remote operation, which creates a lot of network trips. If you have a data structure that is mostly read, then you should consider creating a local Near Cache, so that reads are sped up and less network traffic is created.

These benefits do not come for free, please consider the following trade-offs:

- Clients with a Near Cache will have to hold the extra cached data, which increases memory consumption.

- If invalidation is enabled and entries are updated frequently, then invalidations will be costly.

- Near Cache breaks the strong consistency guarantees; you might be reading stale data.

Near Cache is highly recommended for maps that are mostly read.

In a client/server system you must enable the Near Cache separately on the client, without the need to configure it on the server. Please note that the Near Cache configuration is specific to the server or client itself: a data structure on a server may not have a Near Cache configured while the same data structure on a client may have it configured. They also can have different Near Cache configurations.

If you are using the Near Cache, you should take into account that your hits to the keys in the Near Cache are not reflected as hits to the original keys on the primary members. This has for example an impact on imap's maximum idle seconds or time-to-live seconds expiration. Therefore, even though there is a hit on a key in the Near Cache, your original key on the primary member may expire.

NOTE: Near Cache works only when you access data via the `map->get(k)` method. Data returned using a predicate is not stored in the Near Cache.

A Near Cache can have its own `in-memory-format` which is independent of the `in-memory-format` of the data structure.

#### 7.8.2.1 Configuring Near Cache

Hazelcast Map can be configured to work with near cache enabled. You can enable the Near Cache on a Hazelcast Map by adding its configuration for that map-> An example configuration for `myMap` is shown below.

```c++
client_config config;
const char *mapName = "EvictionPolicyMap";
config::near_cache_config nearCacheConfig(mapName, config::OBJECT);
nearCacheConfig.set_invalidate_on_change(false);
nearCacheConfig.get_eviction_config().set_eviction_policy(config::LRU)
.set_maximum_size_policy(config::eviction_config::ENTRY_COUNT).set_size(100);
config.add_near_cache_config(nearCacheConfig);
hazelcast_client hz{ new_client(std::move(config)).get() };

```

Following are the descriptions of all configuration elements:
 - `set_in_memory_format`: Specifies in which format data will be stored in your Near Cache. Note that a map’s in-memory format can be different from that of its Near Cache. Available values are as follows:
  - `BINARY`: Data will be stored in serialized binary format (default value).
  - `OBJECT`: Data will be stored in deserialized form.
 - `set_invalidate_on_change`: Specifies whether the cached entries are evicted when the entries are updated or removed in members. Its default value is true.
 - `set_time_to_live_seconds`: Maximum number of seconds for each entry to stay in the Near Cache. Entries that are older than this period are automatically evicted from the Near Cache. Regardless of the eviction policy used, `timeToLiveSeconds` still applies. Any integer between 0 and `INT32_MAX`. 0 means infinite. Its default value is 0.
 - `set_max_idle_seconds`: Maximum number of seconds each entry can stay in the Near Cache as untouched (not read). Entries that are not read more than this period are removed from the Near Cache. Any integer between 0 and `INT32_MAX`. 0 means infinite. Its default value is 0. 
 - `set_eviction_config`: Eviction policy configuration. The following can be set on this config:
    - `set_eviction`: Specifies the eviction behavior when you use High-Density Memory Store for your Near Cache. It has the following attributes:
        - `set_eviction_policy`: Eviction policy configuration. Available values are as follows:
            - `LRU`: Least Recently Used (default value).
            - `LFU`: Least Frequently Used.
            - `NONE`: No items will be evicted and the property max-size will be ignored. You still can combine it with `time-to-live-seconds` and `max-idle-seconds` to evict items from the Near Cache.
            - `RANDOM`: A random item will be evicted.
        - `set_max_size_policy`: Maximum size policy for eviction of the Near Cache. Available values are as follows:
            - `ENTRY_COUNT`: Maximum size based on the entry count in the Near Cache (default value).
        - `set_size`: Maximum size of the Near Cache used for `max-size-policy`. When this is reached the Near Cache is evicted based on the policy defined. Any integer between `1` and `INT32_MAX`. Default is `INT32_MAX`.
- `set_local_update_policy`: Specifies the update policy of the local Near Cache. It is available on JCache clients. Available values are as follows:
    - `INVALIDATE`: Removes the Near Cache entry on mutation. After the mutative call to the member completes but before the operation returns to the caller, the Near Cache entry is removed. Until the mutative operation completes, the readers still continue to read the old value. But as soon as the update completes the Near Cache entry is removed. Any threads reading the key after this point will have a Near Cache miss and call through to the member, obtaining the new entry. This setting provides read-your-writes consistency. This is the default setting.
    - `CACHE_ON_UPDATE`: Updates the Near Cache entry on mutation. After the mutative call to the member completes but before the put returns to the caller, the Near Cache entry is updated. So a remove will remove it and one of the put methods will update it to the new value. Until the update/remove operation completes, the entry's old value can still be read from the Near Cache. But before the call completes the Near Cache entry is updated. Any threads reading the key after this point will read the new entry. If the mutative operation was a remove, the key will no longer exist in the cache, both the Near Cache and the original copy in the member. The member will initiate an invalidate event to any other Near Caches, however the caller Near Cache is not invalidated as it already has the new value. This setting also provides read-your-writes consistency.

#### 7.8.2.2. Near Cache Example for Map
The following is an example configuration for a Near Cache defined in the `mostlyReadMap` map-> According to this configuration, the entries are stored as `OBJECT`'s in this Near Cache and eviction starts when the count of entries reaches `5000`; entries are evicted based on the `LRU` (Least Recently Used) policy. In addition, when an entry is updated or removed on the member side, it is eventually evicted on the client side.
```c++
config::near_cache_config nearCacheConfig(mapName, config::OBJECT);
nearCacheConfig.set_invalidate_on_change(false);
nearCacheConfig.get_eviction_config().set_eviction_policy(config::LRU)
        .set_maximum_size_policy(config::eviction_config::ENTRY_COUNT).set_size(100);
```

#### 7.8.2.3. Near Cache Eviction
In the scope of Near Cache, eviction means evicting (clearing) the entries selected according to the given `eviction_policy` when the specified `size` has been reached.

The `eviction_config::set_size` defines the entry count when the Near Cache is full and determines whether the eviction should be triggered. 

Once the eviction is triggered the configured `eviction_policy` determines which, if any, entries must be evicted.
 
#### 7.8.2.4. Near Cache Expiration
Expiration means the eviction of expired records. A record is expired:

- if it is not touched (accessed/read) for `max_idle_seconds`
- `time_to_live_seconds` passed since it is put to Near Cache

The actual expiration is performed when a record is accessed: it is checked if the record is expired or not. If it is expired, it is evicted and the value shall be looked up from the cluster.

#### 7.8.2.5. Near Cache Invalidation

Invalidation is the process of removing an entry from the Near Cache when its value is updated or it is removed from the original map (to prevent stale reads). See the [Near Cache Invalidation section](https://docs.hazelcast.com/imdg/latest/performance/near-cache.html#near-cache-invalidation) in the Hazelcast IMDG Reference Manual.

### 7.8.3. Pipelining

With the pipelining, you can send multiple
requests in parallel using a single thread  and therefore can increase throughput. 
As an example, suppose that the round trip time for a request/response
is 1 millisecond. If synchronous requests are used, e.g., `imap::get()`, then the maximum throughput out of these requests from
a single thread is 1/001 = 1000 operations/second. One way to solve this problem is to introduce multithreading to make
the requests in parallel. For the same example, if we would use 2 threads, then the maximum throughput doubles from 1000
operations/second, to 2000 operations/second.

However, introducing threads for the sake of executing requests isn't always convenient and doesn't always lead to an optimal
performance; this is where the `pipelining` can be used. If you would use 2 asynchronous calls from a single thread,
then the maximum throughput is 2*(1/001) = 2000 operations/second. Therefore, to benefit from the pipelining, asynchronous calls need to
be made from a single thread. The pipelining is a convenience implementation to provide back pressure, i.e., controlling
the number of inflight operations, and it provides a convenient way to wait for all the results.

```c++
constexpr int depth = 10;
auto pipelining = pipelining<std::string>::create(depth);
for (int k = 0; k < 100; ++k) {
  int key = rand() % keyDomain;
  pipelining->add(map->get(key));
}

// wait for completion
auto results = pipelining->results();
```

In the above example, we make 100 asynchronous `map->get()` calls, but the maximum number of inflight calls is 10.

By increasing the debt of the pipelining, throughput can be increased. The pipelining has its own back pressure, you do not
need to enable the [Client BackPressure section](#733-client-backpressure) on the client or member to have this feature on the pipelining. However, if you have many
pipelines, you may still need to enable the client/member back pressure because it is possible to overwhelm the system
with requests in that situation. See the [Client BackPressure section](#733-client-backpressure) to learn how to enable it on the client or member.

You do not need a special configuration, it works out-of-the-box.

The pipelining can be used for any asynchronous call. You can use it for imap asynchronous get/put methods.
It cannot be used as a transaction mechanism though. So you cannot do some calls and throw away the pipeline and expect that
none of the requests are executed. The pipelining is just a performance optimization, not a mechanism for atomic behavior.

The pipelines are cheap and should frequently be replaced because they accumulate results. It is fine to have a few hundred or
even a few thousand calls being processed with the pipelining. However, all the responses to all requests are stored in the pipeline
as long as the pipeline is referenced. So if you want to process a huge number of requests, then every few hundred or few
thousand calls wait for the pipelining results and just create a new instance.

Note that the pipelines are not thread-safe. They must be used by a single thread.

## 7.9. Monitoring and Logging

### 7.9.1. Enabling Client Statistics

You can enable the client statistics before starting your clients. There are two properties related to client statistics:

- `hazelcast.client.statistics.enabled`: If set to `true`, it enables collecting the client statistics and sending them to the cluster. When it is `true` you can monitor the clients that are connected to your Hazelcast cluster, using Hazelcast Management Center. Its default value is `false`.

- `hazelcast.client.statistics.period.seconds`: Period in seconds the client statistics are collected and sent to the cluster. Its default value is `3`.

You can enable client statistics and set a non-default period in seconds as follows:

```c++
hazelcast::client::client_config config;
config.set_property(hazelcast::client::client_properties::STATISTICS_ENABLED, "true");
config.set_property(hazelcast::client::client_properties::STATISTICS_PERIOD_SECONDS, "4");
```

After enabling the client statistics, you can monitor your clients using Hazelcast Management Center. Please refer to the [Monitoring Clients section](https://docs.hazelcast.com/management-center/latest/monitor-imdg/monitor-clients.html) in the Hazelcast Management Center Reference Manual for more information on the client statistics.

### 7.9.2. Logging Configuration

Hazelcast C++ client emits log messages to provide information about important events and errors. The log levels in increasing order of severity are: `FINEST`, `FINER`, `FINE`, `INFO`, `WARNING` and `SEVERE`.

If no logging configuration is made by the user, the client prints log messages of level `INFO` or above to the standard output. 

You can set the minimum level in which the log messages are printed using `logger_config::level`:
```c++
clientConfig.getLoggerConfig().level(hazelcast::logger::level::warning);
```

`hazelcast::logger::level::off` and `hazelcast::logger::level::all` can be used to enable or disable all levels:

```c++
clientConfig.getLoggerConfig().level(hazelcast::logger::level::off); // disables logging completely
clientConfig.getLoggerConfig().level(hazelcast::logger::level::all); // enables all log levels
```

You can set a callback function to be called on each log message via `logger_config::handler`:
```c++
    clientConfig.getLoggerConfig().handler(my_log_handler);
```
Setting a log handler will disable the default log handling behavior. 

The handler takes instance name of the client object that emitted the message, cluster name that the client is connected, name of the source code and the line number where the log was produced, the severity level of the log message, and the log message itself. Here is the exact signature for a log handler function:

```c++
void my_log_handler(
  const std::string &instance_name, // instance name of the client
  const std::string &cluster_name,  // name of the cluster to which the client is connected
  const char *file_name,            // name of the source file where the log was produced
  int line,                         // line number where the log was produced
  level lvl,                        // severity level of the message
  const std::string &msg);          // the message
```

As the callback function is called from multiple threads, it must be thread-safe.

Refer to the examples directory and the API documentation for details.

## 7.10. Mixed Object Types Supporting Hazelcast Client

Sometimes, you may need to use Hazelcast data structures with objects of different types. For example, you may want to put `int`, `string`, identified data serializable, portable serializable, etc. objects into the same Hazelcast `imap` data structure.

### 7.10.1. typed_data API

The typed_data class is a wrapper class for the serialized binary data. It presents the following user APIs:

```
/**
 *
 * @return The type of the underlying object for this binary.
 */
serialization::pimpl::object_type get_type() const;

/**
 * Deserializes the underlying binary data and produces the object of type T.
 *
 * <b>CAUTION</b>: The type that you provide should be compatible with what object type is returned with
 * the get_type API, otherwise you will either get an exception of incorrectly try deserialize the binary data.
 *
 * @tparam T The type to be used for deserialization
 * @return The object instance of type T.
 */
template <typename T>
boost::optional<T> get() const;
```

typed_data does late deserialization of the data only when the get method is called.

This typed_data allows you to retrieve the data type of the underlying binary to be used when being deserialized. This class represents the type of a Hazelcast serializable object. The fields can take the following values:
1. <b>Primitive types</b>: `factory_id=-1`, `class_id=-1`, `type_id` is the type ID for that primitive as listed in `serialization_constants`
2. <b>Array of primitives</b>: `factory_id=-1`, `class_id=-1`, `type_id` is the type ID for that array as listed in `serialization_constants`
3. <b>identified_data_serializer serialized objects</b>: `factory_id`, `class_id` and `type_id` are non-negative values.
4. <b>Portable serialized objects</b>: `factory_id`, `class_id` and `type_id` are non-negative values.
5. <b>Custom serialized objects</b>: `factory_id=-1`, `class_id=-1`, `type_id` is the non-negative type ID. 
## 7.11. SQL Module
To use this service, the Jet engine must be enabled on the members and the `hazelcast-sql` module must be in the classpath of the members.
If you are using the CLI, Docker image, or distributions to start Hazelcast members, then you don't need to do anything, as the above preconditions are already satisfied for such members.

However, if you are using Hazelcast members in the embedded mode, or receiving errors saying that The Jet engine is disabled or `Cannot execute SQL query because "hazelcast-sql" module is not in the classpath.` while executing queries, enable the Jet engine following one of the instructions pointed out in the error message, or add the `hazelcast-sql` module to your member's classpath.
### 7.11.1 Overview
All the sql related types and functionalilities accomodates in `hazelcast::client::sql` namespace. It is possible to execute queries on `IMap`, `Kafka` and `Files` by using this module. 

`sql_service` is the main controller of this module. Queries can be executed via `sql_service` class. It also supports to specify query parameters. After executing any query `boost::future<std::shared_ptr<sql_result>>` is returned.

If it is a `SELECT` query then it provides a way to iterate over rows. Rows are stored in pages and essentially, hazelcast serves result of `SELECT` queries page by page so the interface reflects this approach.

`sql_result` provides an `page_iterator` which allows caller to fetch and iterate over pages. `sql_result::page_iterator::next()` returns `boost::future<std::shared_ptr<sql_page>>`. Every `sql_page` contains either `0` or more rows. It also tells that whether this page is the `last` or not via `last()` method.
### 7.11.2 Create Mapping
Before you can access any object using SQL, a mapping has to be created. See the documentation for the [CREATE MAPPING](https://docs.hazelcast.com/hazelcast/5.1/sql/create-mapping) command.

``` C++
using namespace hazelcast::client::sql;

auto hz = hazelcast::new_client().get();

// populate the map with some data
auto map = hz.get_map("integers").get();
for (int i = 0; i < 1000; ++i) {
    map->put(i, i).get();
}

auto sql = hz.get_sql();
// Create mapping for the integers. This needs to be done only once per map.
auto result = sql.execute(R"(
                CREATE OR REPLACE MAPPING integers
                    TYPE IMap
                    OPTIONS (
                        'keyFormat' = 'int',
                        'valueFormat' = 'int'
                        )
                    )").get();
```
### 7.11.3 Read SELECT Results
`SELECT` results are fetched page by page. A page can accomodate zero or more rows.

`sql_service::execute` executes the query and returns a `boost::future<std::shared_ptr<sql_result>>`. Via `sql_result::iterator()` method, `sql_result::page_iterator` is acquired and pages can be iterated.
``` C++
auto hz = hazelcast::new_client().get();

auto result = hz.get_sql().execute("SELECT * FROM integers").get();

result->row_metadata(); // Tells about metadata

for (auto itr = result->iterator(); itr.has_next();)
{
    auto page = itr.next().get();

    for(const sql_page::sql_row& row : page->rows())
    {
        std::cout << row.get_object<int>(0)
                  << ", "
                  << row.get_object<int>(1)
                  << '\n';
    }
}
```

**Note-1**: First call to `sql_result::page_iterator::next()` function will return `ready` future because first page is already loaded.
``` C++
auto itr = result->iterator();
itr.next().get(); // This will not block even `get()` is called because it will return a future which is in ready state.
```
**Note-2**: After first page every `sql_result::page_iterator::next()` call fetches a new page and it must not be called consecutively.
``` C++
auto itr = result->iterator();
itr.next().get(); // It will return first page
auto page_2 = itr.next(); // Made request to second page
// page_2.get(); // I should have waited but didn't
auto page_3 = itr.next(); // This probably throws an `hazelcast::illegal_access` exception
                          // because previous page might not be fetched yet. So `page_2.get()`
                          // should have called to ensure that page_2 is fetched.
```
**Note-3**: If fetched page is marked as `last` there should not be any further fetch requests.
``` C++
auto itr = result->iterator();

auto page_1 = itr.next().get();

if (page_1->last())
    itr.next(); // This will throw an `hazelcast::no_such_element` exception.
```
**Note-4**: Zero rows do not mean that it is the last page. `SELECT * FROM TABLE(generate_stream(1))` query mostly returns first page as empty because it is a stream.
``` C++
using namespace hazelcast::client::sql;

auto hz = hazelcast::new_client().get();

auto result = sql.execute("SELECT * FROM TABLE(generate_stream(1))").get();

auto itr = result->iterator();
auto page = itr.next().get();

std::cout << page.row_count() << std::endl; // This probably print zero
```
**Note-5**: `future<std::shared_ptr<sql_result>>` is returned from `sql_service::execute` and `boost::future<std::shared_ptr<sql_page>>` is returned from `sql_result::page_iterator::next()` can throw `sql::hazelcast_sql_exception` in case of an error so it should be taken into consideration for production ready code.
``` C++
auto result_f = sql.execute("SELECT * FROM TABLE(generate_stream(1))");

try
{
    result_f.get();
}
catch (const hazelcast_sql_exception&)
{
    // Do error handling
}
```

``` C++
auto result = sql.execute("SELECT * FROM TABLE(generate_stream(1))").get();

try
{
    for (auto itr = result.iterator(); itr.has_next();)
    {
        auto page_f = itr.next();
        auto page = page_f.get();
    }
}
catch (const hazelcast_sql_exception&)
{
    // Do error handling
}
```

### 7.11.4 Sql Statement With Options
`sql_statement` is a class which holds query string, its parameters and options.
It can be passed to `sql_service::execute` method and its parameters and options will be taken into consideration.

``` C++
using namespace hazelcast::client::sql;

auto hz = hazelcast::new_client().get();

// Constructs sql_statement with default options which are :
// cursor_buffer_size: 4096
// timeout: std::chrono::millisecond{ -1 } which means no timeout
// expected_result_type: any
sql_statement statement
{
    hz ,
    R"(
        SELECT * FROM some_table WHERE this > ?
    )"
};

// Methods which specifies options are able to be called in chain, returns self references
statement.add_parameter(15); // '?' part of the query will be filled with this parameter.
         .cursor_buffer_size(96); // Set '96' to 'cursor_buffer_size' option
         .timeout(std::chrono::millisecond {500}); // Set '500' milliseconds timeout, if the time is exceeded query will be cancelled.
         .expected_result_type(sql_expected_result_type::rows); // Expect a table which contains rows

// To fetch use non-parameterized overloads
std::cout << "cursor_buffer_size : " << statement.cursor_buffer_size() << std::endl
          << "timeout : " << statement.timeout() << std::endl
          << "expected_result_type : " << int(statement.expected_result_type()) << std::endl;

hz.get_sql().execute(statement); // OK, execute this statement
```

### 7.11.5 Read Row Metadata
Row metadatas can be acquired with `sql_result::row_metadata()` method.

``` C++
auto result = sql.execute("SELECT name, surname, age FROM employees").get();

auto metadata = result->row_metadata();

// Print column count
std::cout << "There are " << metadata.column_count() << " columns" << std::endl;

// Print columns
for (const sql_column_metadata& col : metadata.columns())
{
    std::cout << "name : "      << col.name
              << " type : "     << col.type
              << " nullable : " << col.nullable
              << std::endl;
}

// Find columny by name
auto surname_col_metadata = *(metadata.find_column("surname"));

std::cout << "name : "      << surname_col_metadata.name
          << " type : "     << surname_col_metadata.type
          << " nullable : " << surname_col_metadata.nullable
          << std::endl;

// Get column by index
auto age_col_metadata = *(metadata.column(2));

std::cout << "name : "      << age_col_metadata.name
          << " type : "     << age_col_metadata.type
          << " nullable : " << age_col_metadata.nullable
          << std::endl;
```
**Note :** There is no metadata for non-`SELECT` queries.

``` C++
auto result = sql.execute("DELETE FROM employees WHERE age < 18").get();

result->row_metadata(); // Throws an `exception::illegal_state`
                        // This is not a SELECT query
```

# 8. Development and Testing

Hazelcast C++ client is developed using C++. If you want to help with bug fixes, develop new features or
tweak the implementation to your application's needs, you can follow the steps in this section.

## 8.1. Testing

In order to test Hazelcast C++ client locally, you will need the following:

* Java 8 or newer (for server)
* Maven
* cmake
* OpenSSL
* Boost
* thrift

You need to enable the examples in the build with the cmake flag `-DBUILD_EXAMPLES=ON`. When you build, the result will
produce the executable `client_test` on Linux/macOS and `client_test.exe` on Windows. You need to start the remote
controller first before running this executable by executing `scripts/start-rc.sh` on Linux/macOS
or `scripts/start-rc.bat` on Windows. Then you can run the test executable.

# 9. Getting Help

You can use the following channels for your questions and development/usage issues:

* This repository by opening an issue.
* [Hazelcast Community Slack - C++ Client Channel](https://hazelcastcommunity.slack.com/channels/cpp-client)
* Google Groups: https://groups.google.com/forum/#!forum/hazelcast
* Stack Overflow: https://stackoverflow.com/questions/tagged/hazelcast

# 10. Contributing

Besides your development contributions as explained in the [Development and Testing chapter](#8-development-and-testing) above, you can always open a pull request on this repository for your other requests such as documentation changes.

# 11. License

[Apache 2 License](https://github.com/hazelcast/hazelcast-cpp-client/blob/master/LICENSE).

# 12. Copyright

Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.

Visit [www.hazelcast.com](http://www.hazelcast.com) for more information.
