# Table of Contents

* [Introduction](#introduction)
* [1. Getting Started](#1-getting-started)
  * [1.1. Requirements](#11-requirements)
  * [1.2. Working with Hazelcast IMDG Clusters](#12-working-with-hazelcast-imdg-clusters)
    * [1.2.1. Setting Up a Hazelcast IMDG Cluster](#121-setting-up-a-hazelcast-imdg-cluster)
      * [1.2.1.1. Running Standalone JARs](#1211-running-standalone-jars)
      * [1.2.1.2. Adding User Library to CLASSPATH](#1212-adding-user-library-to-classpath)
      * [1.2.1.3. Using hazelcast-member Tool](#1213-using-hazelcast-member-tool)
  * [1.3. Downloading and Installing](#13-downloading-and-installing)
  * [1.4. Basic Configuration](#14-basic-configuration)
    * [1.4.1. Configuring Hazelcast IMDG](#141-configuring-hazelcast-imdg)
    * [1.4.2. Configuring Hazelcast C++ Client](#142-configuring-hazelcast-cpp-client)
        * [1.4.2.1. Group Settings](#1421-group-settings)
        * [1.4.2.2. Network Settings](#1422-network-settings)
    * [1.4.3. Client System Properties](#143-client-system-properties)        
  * [1.5. Basic Usage](#15-basic-usage)
  * [1.6. Code Samples](#16-code-samples)
* [2. Features](#2-features)
* [3. Configuration Overview](#3-configuration-overview)
  * [3.1. Configuration Options](#31-configuration-options)
    * [3.1.1. Programmatic Configuration](#311-programmatic-configuration)
* [4. Serialization](#4-serialization)
  * [4.1. IdentifiedDataSerializable Serialization](#41-identifieddataserializable-serialization)
  * [4.2. Portable Serialization](#42-portable-serialization)
  * [4.3. Custom Serialization](#43-custom-serialization)
  * [4.4. Global Serialization](#44-global-serialization)
* [5. Setting Up Client Network](#5-setting-up-client-network)
  * [5.1. Providing Member Addresses](#51-providing-member-addresses)
  * [5.2. Setting Smart Routing](#52-setting-smart-routing)
  * [5.3. Enabling Redo Operation](#53-enabling-redo-operation)
  * [5.4. Setting Connection Timeout](#54-setting-connection-timeout)
  * [5.5. Setting Connection Attempt Limit](#55-setting-connection-attempt-limit)
  * [5.6. Setting Connection Attempt Period](#56-setting-connection-attempt-period)
  * [5.7. Enabling Client TLS/SSL](#57-enabling-client-tlsssl)
  * [5.8. Enabling Hazelcast AWS Cloud Discovery](#58-enabling-hazelcast-aws-cloud-discovery)
* [6. Securing Client Connection](#6-securing-client-connection)
  * [6.1. TLS/SSL](#61-tlsssl)
    * [6.1.1. TLS/SSL for Hazelcast Members](#611-tlsssl-for-hazelcast-members)
    * [6.1.2. TLS/SSL for Hazelcast C++ Clients](#612-tlsssl-for-hazelcast-cpp-clients)
    * [6.1.3. Mutual Authentication](#613-mutual-authentication)
* [7. Using C++ Client with Hazelcast IMDG](#7-using-cpp-client-with-hazelcast-imdg)
  * [7.1. C++ Client API Overview](#71-cpp-client-api-overview)
  * [7.2. C++ Client Operation Modes](#72-cpp-client-operation-modes)
      * [7.2.1. Smart Client](#721-smart-client)
      * [7.2.2. Unisocket Client](#722-unisocket-client)
  * [7.3. Handling Failures](#73-handling-failures)
    * [7.3.1. Handling Client Connection Failure](#731-handling-client-connection-failure)
    * [7.3.2. Handling Retry-able Operation Failure](#732-handling-retry-able-operation-failure)    
    * [7.3.3. Client Backpressure](#733-client-backpressure)
    * [7.3.4. Client Connection Strategy](#734-client-connection-strategy)
        * [7.3.4.1. Configuring Client Reconnect Strategy](#7341-configuring-client-reconnect-strategy)
  * [7.4. Using Distributed Data Structures](#74-using-distributed-data-structures)
    * [7.4.1. Using Map](#741-using-map)
    * [7.4.2. Using MultiMap](#742-using-multimap)
    * [7.4.3. Using Replicated Map](#743-using-replicated-map)
    * [7.4.4. Using Queue](#744-using-queue)
    * [7.4.5. Using Set](#745-using-set)
    * [7.4.6. Using List](#746-using-list)
    * [7.4.7. Using Ringbuffer](#747-using-ringbuffer)
    * [7.4.8. Using Reliable Topic](#748-using-reliable-topic) 
    * [7.4.9. Using Lock](#749-using-lock)
    * [7.4.10. Using Atomic Long](#7410-using-atomic-long)
    * [7.4.11. Using Semaphore](#7411-using-semaphore)
    * [7.4.12. Using PN Counter](#7412-using-pn-counter)
    * [7.4.13. Using Flake ID Generator](#7413-using-flake-id-generator)
    * [7.4.14. Using Transactions](#7414-using-transactions)
  * [7.5. Distributed Events](#75-distributed-events)
    * [7.5.1. Cluster Events](#751-cluster-events)
      * [7.5.1.1. Listening for Member Events](#7511-listening-for-member-events)
      * [7.5.1.2. Listening for Distributed Object Events](#7512-listening-for-distributed-object-events)
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
        * [7.6.1.6 Callback When Task Completes](#7616-callback-when-task-completes)
            * [7.6.1.6.1 Example Task to Callback](#76161-example-task-to-callback)
        * [7.6.1.7 Selecting Members for Task Execution](#7617-selecting-members-for-task-execution)
    * [7.6.2. Using EntryProcessor](#762-using-entryprocessor)
  * [7.7. Distributed Query](#77-distributed-query)
    * [7.7.1. How Distributed Query Works](#771-how-distributed-query-works)
      * [7.7.1.1. Employee Map Query Example](#7711-employee-map-query-example)
      * [7.7.1.2. Querying by Combining Predicates with AND, OR, NOT](#7712-querying-by-combining-predicates-with-and-or-not)
      * [7.7.1.3. Querying with SQL](#7713-querying-with-sql)
      * [7.7.1.4. Filtering with Paging Predicates](#7714-filtering-with-paging-predicates)
  * [7.8. Performance](#78-performance)
      * [7.8.1. Partition Aware](#781-partition-aware)
      * [7.8.2. Near Cache](#782-near-cache)
          * [7.8.2.1. Configuring Near Cache](#7821-configuring-near-cache)
          * [7.8.2.2. Near Cache Example for Map](#7822-near-cache-example-for-map)
          * [7.8.2.3. Near Cache Eviction](#7823-near-cache-eviction)
          * [7.8.2.4. Near Cache Expiration](#7824-near-cache-expiration)
          * [7.8.2.5. Near Cache Invalidation](#7825-near-cache-invalidation)
  * [7.9. Monitoring and Logging](#79-monitoring-and-logging)
      * [7.9.1. Enabling Client Statistics](#791-enabling-client-statistics)
      * [7.9.2. Logging Configuration](#792-logging-configuration)
  * [7.10 Raw Pointer API](#710-raw-pointer-api)
  * [7.11 Mixed Object Types Supporting HazelcastClient](#711-mixed-object-types-supporting-hazelcastclient)
    * [7.11.1 TypedData API](#7111-typeddata-api) 
* [8. Development and Testing](#8-development-and-testing)
  * [8.1. Building and Using Client From Sources](#81-building-and-using-client-from-sources)
      * [8.1.1 Mac](#811-mac)
      * [8.1.2 Linux](#812-linux)
      * [8.1.3 Windows](#813-windows)
  * [8.2. Testing](#82-testing)
    * [8.2.1 Tested Platforms](#821-tested-platforms)
  * [8.3. Reproducing Released Libraries](#83-reproducing-released-libraries)
* [9. Getting Help](#9-getting-help)
* [10. Contributing](#10-contributing)
* [11. License](#11-license)
* [12. Copyright](#12-copyright)


# Introduction

This document provides information about the C++ client for [Hazelcast](https://hazelcast.org/). This client uses Hazelcast's [Open Client Protocol](https://hazelcast.org/documentation/#open-binary) and works with Hazelcast IMDG 3.6 and higher versions.

### Resources

See the following for more information on Hazelcast IMDG:

* Hazelcast IMDG [website](https://hazelcast.org/)
* Hazelcast IMDG [Reference Manual](https://hazelcast.org/documentation/#imdg)

### Release Notes

See the [Releases](https://github.com/hazelcast/hazelcast-cpp-client/releases) page of this repository.


# 1. Getting Started

This chapter provides information on how to get started with your Hazelcast C++ client. It outlines the requirements, installation and configuration of the client, setting up a cluster, and provides a simple application that uses a distributed map in C++ client.

## 1.1. Requirements

- Windows, Linux or MacOS
- Java 6 or newer
- Hazelcast IMDG 3.6 or newer
- Latest Hazelcast C++ Client

## 1.2. Working with Hazelcast IMDG Clusters

Hazelcast C++ client requires a working Hazelcast IMDG cluster to run. This cluster handles storage and manipulation of the user data.
Clients are a way to connect to the Hazelcast IMDG cluster and access such data.

Hazelcast IMDG cluster consists of one or more cluster members. These members generally run on multiple virtual or physical machines
and are connected to each other via network. Any data put on the cluster is partitioned to multiple members transparent to the user.
It is therefore very easy to scale the system by adding new members as the data grows. Hazelcast IMDG cluster also offers resilience. Should
any hardware or software problem causes a crash to any member, the data on that member is recovered from backups and the cluster
continues to operate without any downtime. Hazelcast clients are an easy way to connect to a Hazelcast IMDG cluster and perform tasks on
distributed data structures that live on the cluster.

In order to use Hazelcast C++ client, we first need to setup a Hazelcast IMDG cluster.

### 1.2.1. Setting Up a Hazelcast IMDG Cluster

There are following options to start a Hazelcast IMDG cluster easily:

* You can run standalone members by downloading and running JAR files from the website.
* You can embed members to your Java projects. 
* The easiest way is to use [hazelcast-member tool](https://github.com/hazelcast/hazelcast-member-tool) if you have brew installed in your computer.

We are going to download JARs from the website and run a standalone member for this guide.

#### 1.2.1.1. Running Standalone JARs

Follow the instructions below to create a Hazelcast IMDG cluster:

1. Go to Hazelcast's download [page](https://hazelcast.org/download/) and download either the `.zip` or `.tar` distribution of Hazelcast IMDG.
2. Decompress the contents into any directory that you
want to run members from.
3. Change into the directory that you decompressed the Hazelcast content and then into the `bin` directory.
4. Use either `start.sh` or `start.bat` depending on your operating system. Once you run the start script, you should see the Hazelcast IMDG logs in the terminal.

You should see a log similar to the following, which means that your 1-member cluster is ready to be used:

```
INFO: [192.168.0.3]:5701 [dev] [3.10.4]

Members {size:1, ver:1} [
	Member [192.168.0.3]:5701 - 65dac4d1-2559-44bb-ba2e-ca41c56eedd6 this
]

Sep 06, 2018 10:50:23 AM com.hazelcast.core.LifecycleService
INFO: [192.168.0.3]:5701 [dev] [3.10.4] [192.168.0.3]:5701 is STARTED
```

#### 1.2.1.2. Adding User Library to CLASSPATH

When you want to use features such as querying and language interoperability, you might need to add your own Java classes to the Hazelcast member in order to use them from your C++ client. This can be done by adding your own compiled code to the `CLASSPATH`. To do this, compile your code with the `CLASSPATH` and add the compiled files to the `user-lib` directory in the extracted `hazelcast-<version>.zip` (or `tar`). Then, you can start your Hazelcast member by using the start scripts in the `bin` directory. The start scripts will automatically add your compiled classes to the `CLASSPATH`.

Note that if you are adding an `IdentifiedDataSerializable` or a `Portable` class, you need to add its factory too. Then, you should configure the factory in the `hazelcast.xml` configuration file. This file resides in the `bin` directory where you extracted the `hazelcast-<version>.zip` (or `tar`).

The following is an example configuration when you are adding an `IdentifiedDataSerializable` class:

```xml
<hazelcast>
     ...
     <serialization>
        <data-serializable-factories>
            <data-serializable-factory factory-id=<identified-factory-id>>
                IdentifiedFactoryClassName
            </data-serializable-factory>
        </data-serializable-factories>
    </serialization>
    ...
</hazelcast>
```
If you want to add a `Portable` class, you should use `<portable-factories>` instead of `<data-serializable-factories>` in the above configuration.

#### 1.2.1.3. Using hazelcast-member Tool

`hazelcast-member` is a tool to download and run Hazelcast IMDG members easily. You can find the installation instructions for various platforms in the following sections.

##### Installing on Mac OS X

If you have brew installed, run the following commands to install this tool:

```
brew tap hazelcast/homebrew-hazelcast
brew install hazelcast-member
```

##### Installing on Ubuntu and Debian

To resolve the `.deb` artifacts from Bintray, follow the below instructions.

First, you need to import the Bintray's GPG key using the following command:

```
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 379CE192D401AB61
```

Then, run the following commands to add the `.deb` artifact to your system configuration file and update the lists of packages:

```
echo "deb https://dl.bintray.com/hazelcast/deb stable main" | sudo tee -a /etc/apt/sources.list
sudo apt-get update
``` 

Finally, run the following command to install the `hazelcast-member` tool:

```
sudo apt-get install hazelcast-member
```

##### Installing on Red Hat and CentOS

To resolve the `RPM` artifacts from Bintray, follow the below instructions.

First, run the following command to get a generated `.repo` file:

```
wget https://bintray.com/hazelcast/rpm/rpm -O bintray-hazelcast-rpm.repo
```

Then, install the `.repo` file using the following command:


```
sudo mv bintray-hazelcast-rpm.repo /etc/yum.repos.d/
```

Finally, run the following command to install the `hazelcast-member` tool:

```
sudo yum install hazelcast-member
```

---

After successfully installing the `hazelcast-member` tool, you can start a member by running the following command:

```
hazelcast-member start
```

To stop a member, run the following command:

```
hazelcast-member stop
```

You can find more information about the `hazelcast-member` tool at its GitHub [repo](https://github.com/hazelcast/hazelcast-member-tool).

See the [Hazelcast IMDG Reference Manual](http://docs.hazelcast.org/docs/latest/manual/html-single/index.html#getting-started) for more information on setting up the clusters.

## 1.3. Downloading and Installing

Download the latest C++ client library from [Hazelcast C++ Client Website](https://hazelcast.org/clients/cplusplus/). You need to download the zip file for your platform. For Linux and Windows, 32- and 64-bit libraries exist. For MacOS, there is only 64-bit version. 

Unzip the file. Following is the directory structure for Linux 64-bit zip. The structure is similar for the other C++ client distributions.

- `cpp/Linux_64`
    - `hazelcast`:
        - `lib`: Shared and static library directory.
            - `tls`: Contains the library with TLS (SSL) support enabled.
        - `include`: Directory you need to include when compiling your project.
    - `external/include`: External directory that you need to include when compiling your project. Currently the only dependency is `boost/shared_ptr.hpp`.
        - `boost`: External boost files for `boost/shared_ptr`.
    - `examples`: Contains various examples for each C++ client feature. Each example produces an executable which you can run in a cluster. You may need to set the server IP addresses for the examples to run.
    
### 1.3.1 Compiling Your Project

For compilation, you need to include the `hazelcast/include` and `external/include` directories in your in your distribution. You also need to link your application to the appropriate static or shared library. 

If you want to use the TLS feature, use the `lib` directory with TLS support enabled, e.g., `cpp/Linux_64/hazelcast/lib/tls`.

#### 1.3.1.1 Mac Client

For Mac, there is only 64-bit distribution.

Here is an example script to build with the static library:

`g++ main.cpp -Icpp/Mac_64/hazelcast/external/include -Icpp/Mac_64/hazelcast/include cpp/Mac_64/hazelcast/lib/libHazelcast3.10.1_64.a`

Here is an example script to build with the shared library:

`g++ main.cpp -Icpp/Mac_64/hazelcast/external/include -Icpp/Mac_64/hazelcast/include -Lcpp/Mac_64/hazelcast/lib -lHazelcastClient3.10.1_64`

#### 1.3.1.2 Linux Client

For Linux, there are 32- and 64-bit distributions. You need to link with pthread library when compiling in Linux.

Here is an example script to build with the static library:

`g++ main.cpp -pthread -Icpp/Linux_64/hazelcast/external/include -Icpp/Linux_64/hazelcast/include
      cpp/Linux_64/hazelcast/lib/libHazelcastClient3.10.1_64.a`

Here is an example script to build with the shared library:

`g++ main.cpp -lpthread -Wl,–no-as-needed -lrt -Icpp/Linux_64/hazelcast/external/include -Icpp/Linux_64/hazelcast/include -Lcpp/Linux_64/hazelcast/lib -lHazelcastClient3.10.1_64`

Please add the `__STDC_LIMIT_MACROS` and `__STDC_CONSTANT_MACROS` compilation flags for the environments for which the compilation fails with the error `INT32_MAX could not be determined`. The following is an example command to add these flags:

`g++ main.cpp -pthread -Icpp/Linux_64/hazelcast/external/include -Icpp/Linux_64/hazelcast/include -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS cpp/Linux_64/hazelcast/lib/libHazelcastClient3.10.1_64.a`


#### 1.3.1.3 Windows Client

For Windows, there are 32- and 64-bit distributions. The static library is in the `static` directory and the dynamic library (`dll`) is in the `shared` directory.

When compiling for Windows environment, you should specify one of the following flags:

- `HAZELCAST_USE_STATIC`: You want the application to use the static Hazelcast library.
- `HAZELCAST_USE_SHARED`: You want the application to use the shared Hazelcast library.

## 1.4. Basic Configuration

If you are using Hazelcast IMDG and C++ Client on the same computer, generally the default configuration should be fine. This is great for
trying out the client. However, if you run the client on a different computer than any of the cluster members, you may
need to do some simple configurations such as specifying the member addresses.

The Hazelcast IMDG members and clients have their own configuration options. You may need to reflect some of the member side configurations on the client side to properly connect to the cluster.

This section describes the most common configuration elements to get you started in no time.
It discusses some member side configuration options to ease the understanding of Hazelcast's ecosystem. Then, the client side configuration options
regarding the cluster connection are discussed. The configurations for the Hazelcast IMDG data structures that can be used in the C++ client are discussed in the following sections.

See the [Hazelcast IMDG Reference Manual](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html) and [Configuration Overview section](#3-configuration-overview) for more information.

### 1.4.1. Configuring Hazelcast IMDG

Hazelcast IMDG aims to run out-of-the-box for most common scenarios. However if you have limitations on your network such as multicast being disabled,
you may have to configure your Hazelcast IMDG members so that they can find each other on the network. Also, since most of the distributed data structures are configurable, you may want to configure them according to your needs. We will show you the basics about network configuration here.

You can use the following options to configure Hazelcast IMDG:

* Using the `hazelcast.xml` configuration file.
* Programmatically configuring the member before starting it from the Java code.

Since we use standalone servers, we will use the `hazelcast.xml` file to configure our cluster members.

When you download and unzip `hazelcast-<version>.zip` (or `tar`), you see the `hazelcast.xml` in the `bin` directory. When a Hazelcast member starts, it looks for the `hazelcast.xml` file to load the configuration from. A sample `hazelcast.xml` is shown below.

```xml
<hazelcast>
    <group>
        <name>dev</name>
        <password>dev-pass</password>
    </group>
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

- `<group>`: Specifies which cluster this member belongs to. A member connects only to the other members that are in the same group as
itself. As shown in the above configuration sample, there are `<name>` and `<password>` tags under the `<group>` element with some pre-configured values. You may give your clusters different names so that they can
live in the same network without disturbing each other. Note that the cluster name should be the same across all members and clients that belong
 to the same cluster. The `<password>` tag is not in use since Hazelcast 3.9. It is there for backward compatibility
purposes. You can remove or leave it as it is if you use Hazelcast 3.9 or later.
- `<network>`
    - `<port>`: Specifies the port number to be used by the member when it starts. Its default value is 5701. You can specify another port number, and if
     you set `auto-increment` to `true`, then Hazelcast will try the subsequent ports until it finds an available port or the `port-count` is reached.
    - `<join>`: Specifies the strategies to be used by the member to find other cluster members. Choose which strategy you want to
    use by setting its `enabled` attribute to `true` and the others to `false`.
        - `<multicast>`: Members find each other by sending multicast requests to the specified address and port. It is very useful if IP addresses
        of the members are not static.
        - `<tcp>`: This strategy uses a pre-configured list of known members to find an already existing cluster. It is enough for a member to
        find only one cluster member to connect to the cluster. The rest of the member list is automatically retrieved from that member. We recommend
        putting multiple known member addresses there to avoid disconnectivity should one of the members in the list is unavailable at the time
        of connection.

These configuration elements are enough for most connection scenarios. Now we will move onto the configuration of the C++ client.

### 1.4.2. Configuring Hazelcast C++ Client

You can configure Hazelcast C++ Client programatically.

This section describes some network configuration settings to cover common use cases in connecting the client to a cluster. See the [Configuration Overview section](#3-configuration-overview)
and the following sections for information about detailed network configurations and/or additional features of Hazelcast C++ client configuration.

An easy way to configure your Hazelcast C++ Client is to create a `ClientConfig` object and set the appropriate options. Then you need to
pass this object to the client when starting it, as shown below.

```C++
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config); // Connects to the cluster
```

If you run the Hazelcast IMDG members in a different server than the client, you most probably have configured the members' ports and cluster
names as explained in the previous section. If you did, then you need to make certain changes to the network settings of your client.

### 1.4.2.1 Group Settings

You need to provide the group name of the cluster, if it is defined on the server side, to which you want the client to connect.

```C++
    hazelcast::client::ClientConfig config;
    config.getGroupConfig().setName("group name of your cluster");
```

### 1.4.2.2. Network Settings

You need to provide the IP address and port of at least one member in your cluster so the client can find it.

```C++
    hazelcast::client::ClientConfig config;
    config.getNetworkConfig().addAddress(hazelcast::client::Address("your server ip", 5701 /* your server port*/));
```
### 1.4.3. Client System Properties

While configuring your C++ client, you can use various system properties provided by Hazelcast to tune its clients. These properties can be set programmatically through `config.SetProperty` or by using an environment variable.
The value of this property will be:

* the programmatically configured value, if programmatically set,
* the environment variable value, if the environment variable is set,
* the default value, if none of the above is set.

See the following for an example client system property configuration:

**Programmatically:**

```C++
config.setProperty(hazelcast::client::ClientProperties::INVOCATION_TIMEOUT_SECONDS, "2") // Sets invocation timeout as 2 seconds
```

or 

```C++
config.SetProperty("hazelcast.client.invocation.timeout.seconds", "2") // Sets invocation timeout as 2 seconds
```

**By using an environment variable on Linux:** 

```C++
export hazelcast.client.invocation.timeout.seconds=2
```

If you set a property both programmatically and via an environment variable, the programmatically
set value will be used.

See the [complete list of system properties](https://github.com/hazelcast/hazelcast-cpp-client/blob/master/hazelcast/include/hazelcast/client/ClientProperties.h), along with their descriptions, which can be used to configure your Hazelcast C++ client.

## 1.5. Basic Usage

Now that we have a working cluster and we know how to configure both our cluster and client, we can run a simple program to use a
distributed map in C++ client.

The following example first creates a programmatic configuration object. Then, it starts a client.

```C++
#include <hazelcast/client/HazelcastClient.h>
int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config); // Connects to the cluster
    std::cout << "Started the Hazelcast C++ client instance " << hz.getName() << std::endl; // Prints client instance name
    return 0;
}
```
This should print logs about the cluster members and information about the client itself such as client type and local address port.
```
24/10/2018 13:54:06.390 INFO: [0x7fff97559340] hz.client_1[dev] [3.10.2-SNAPSHOT] (20181023:5f1a045da7) LifecycleService::LifecycleEvent STARTING
24/10/2018 13:54:06.390 INFO: [0x7fff97559340] hz.client_1[dev] [3.10.2-SNAPSHOT] LifecycleService::LifecycleEvent STARTED
24/10/2018 13:54:06.392 INFO: [0x700006a44000] hz.client_1[dev] [3.10.2-SNAPSHOT] Trying to connect to Address[127.0.0.1:5701] as owner member
24/10/2018 13:54:06.395 INFO: [0x7000069c1000] hz.client_1[dev] [3.10.2-SNAPSHOT] Setting ClientConnection{alive=1, connectionId=1, remoteEndpoint=Address[localhost:5701], lastReadTime=2018-10-24 10:54:06.394000, closedTime=never, connected server version=3.11-SNAPSHOT} as owner with principal uuid: 160e2baf-566b-4cc3-ab25-a1b3e34de320 ownerUuid: 7625eae9-4f8e-4285-82c0-be2f2eee2a50
24/10/2018 13:54:06.395 INFO: [0x7000069c1000] hz.client_1[dev] [3.10.2-SNAPSHOT] Authenticated with server Address[localhost:5701], server version:3.11-SNAPSHOT Local address: Address[127.0.0.1:56007]
24/10/2018 13:54:06.495 INFO: [0x700006cd3000] hz.client_1[dev] [3.10.2-SNAPSHOT] 

Members [2]  {
	Member[localhost]:5702 - 44cf4017-5355-4859-9bf5-fd374d7fc69c
	Member[localhost]:5701 - 7625eae9-4f8e-4285-82c0-be2f2eee2a50
}

24/10/2018 13:54:06.600 INFO: [0x700006a44000] hz.client_1[dev] [3.10.2-SNAPSHOT] LifecycleService::LifecycleEvent CLIENT_CONNECTED
Started the Hazelcast C++ client instance hz.client_1
```
Congratulations! You just started a Hazelcast C++ Client.

**Using a Map**

Let's manipulate a distributed map on a cluster using the client.

Save the following file as `IT.cpp` and compile it using a command similar to the following (Linux g++ compilation is used for demonstration):

```C++
g++ IT.cpp -o IT -lpthread -Icpp/Linux_64/hazelcast/external/include -Icpp/Linux_64/hazelcast/include -Lcpp/Linux_64/hazelcast/lib -lHazelcastClient3.10.1_64
```
Then, you can run the application using the following command:
 
```
./IT
```

**main.cpp**

```C++
#include <hazelcast/client/HazelcastClient.h>
int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config); // Connects to the cluster

    hazelcast::client::IMap<std::string, std::string> personnel = hz.getMap<std::string, std::string>("personnelMap");
    personnel.put("Alice", "IT");
    personnel.put("Bob", "IT");
    personnel.put("Clark", "IT");
    std::cout << "Added IT personnel. Logging all known personnel" << std::endl;
    std::vector<std::pair<std::string, std::string> > entries = personnel.entrySet();
    for (std::vector<std::pair<std::string, std::string> >::const_iterator it = entries.begin();
         it != entries.end(); ++it) {
        std::cout << it->first << " is in " << it->second << " department." << std::endl;
    }
    
    return 0;
}
```

**Output**

```
24/10/2018 13:54:06.600 INFO: [0x700006a44000] hz.client_1[dev] [3.10.2-SNAPSHOT] LifecycleService::LifecycleEvent CLIENT_CONNECTED
Added IT personnel. Logging all known personnel
Alice is in IT department
Clark is in IT department
Bob is in IT department
```

You see this example puts all the IT personnel into a cluster-wide `personnelMap` and then prints all the known personnel.

Now create a `Sales.cpp` file, compile and run it as shown below.

**Compile:**

```C++
g++ Sales.cpp -o Sales -lpthread -Icpp/Linux_64/hazelcast/external/include -Icpp/Linux_64/hazelcast/include -Lcpp/Linux_64/hazelcast/lib -lHazelcastClient3.10.1_64
```
**Run**

Then, you can run the application using the following command:

```
./Sales
```

**Sales.cpp**

```C++
#include <hazelcast/client/HazelcastClient.h>
int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config); // Connects to the cluster

    hazelcast::client::IMap<std::string, std::string> personnel = hz.getMap<std::string, std::string>("personnelMap");
    personnel.put("Denise", "Sales");
    personnel.put("Erwing", "Sales");
    personnel.put("Fatih", "Sales");
    personnel.put("Bob", "IT");
    personnel.put("Clark", "IT");
    std::cout << "Added Sales personnel. Logging all known personnel" << std::endl;
    std::vector<std::pair<std::string, std::string> > entries = personnel.entrySet();
    for (std::vector<std::pair<std::string, std::string> >::const_iterator it = entries.begin();
         it != entries.end(); ++it) {
        std::cout << it->first << " is in " << it->second << " department." << std::endl;
    }

    return 0;
}
```

**Output**

```
24/10/2018 13:54:06.600 INFO: [0x700006a44000] hz.client_1[dev] [3.10.2-SNAPSHOT] LifecycleService::LifecycleEvent CLIENT_CONNECTED
Added Sales personnel. Logging all known personnel
Denise is in Sales department
Erwing is in Sales department
Faith is in Sales department
Alice is in IT department
Clark is in IT department
Bob is in IT department
```

You will see this time we add only the sales employees but we get the list all known employees including the ones in IT.
That is because our map lives in the cluster and no matter which client we use, we can access the whole map.

## 1.6. Code Samples

See the Hazelcast C++ [code samples](https://github.com/hazelcast/hazelcast-cpp-client/tree/master/examples) for more examples.

You can also see the Hazelcast C++ client [API Documentation](https://docs.hazelcast.org/docs/clients/cpp/3.10.1/html/).

# 2. Features

Hazelcast C++ client supports the following data structures and features:

* Map
* Queue
* Set
* List
* MultiMap
* Replicated Map
* Ringbuffer
* Reliable Topic
* Lock
* Semaphore
* Atomic Long
* CRDT PN Counter
* Flake ID Generator
* Event Listeners
* Distributed Executor Service
* Entry Processor
* Transactional Map
* Transactional MultiMap
* Transactional Queue
* Transactional List
* Transactional Set
* Query (Predicates)
* Paging Predicate
* Built-in Predicates
* Listener with Predicate
* Near Cache Support
* Programmatic Configuration
* Fail Fast on Invalid Configuration
* SSL Support (requires Enterprise server)
* Authorization
* Smart Client
* Unisocket Client
* Lifecycle Service
* IdentifiedDataSerializable Serialization
* Portable Serialization
* Custom Serialization
* Global Serialization

# 3. Configuration Overview

This chapter describes the options to configure your C++ client.

## 3.1. Configuration Options

You can configure Hazelcast C++ client programmatically (API).

### 3.1.1. Programmatic Configuration

For programmatic configuration of the Hazelcast C++ client, just instantiate a `ClientConfig` object and configure the
desired aspects. An example is shown below.

```C++
    hazelcast::client::ClientConfig config;
    config.getNetworkConfig().addAddress(hazelcast::client::Address("your server ip", 5701 /* your server port*/));
    hazelcast::client::HazelcastClient hz(config); // Connects to the cluster
```

See the `ClientConfig` class reference at the Hazelcast C++ client [API Documentation](https://docs.hazelcast.org/docs/clients/cpp/3.10.1/html/classhazelcast_1_1client_1_1_client_config.html) for details.

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

If you want the serialization to work faster or you use the clients in different languages, Hazelcast offers its own native serialization types, such as [`IdentifiedDataSerializable` Serialization](#41-identifieddataserializable-serialization) and [`Portable` Serialization](#42-portable-serialization).

On top of all, if you want to use your own serialization type, you can use a [Custom Serialization](#43-custom-serialization).

When Hazelcast serializes an object into Data (byte array):

1. It first checks whether the object pointer is NULL.

2. If the above check fails, then the object is serialized using the serializer that matches the object type. The object type is determined using the free function `int32_t getHazelcastTypeId(const T *object);`. This method returns the constant built-int serializer type ID for the built-in types such as `byte`, `char`, `int32_t`, `bool` and `int64_t`, etc. If it does not match any of the built-in types, it may match the types offered by Hazelcast, i.e., IdentifiedDataSerializable, Portable or custom. The matching is done based on method parameter matching. 

3. If the above check fails, Hazelcast will use the registered Global Serializer if one exists.

If all the above fails, then `exception::HazelcastSerializationException` is thrown.

## 4.1. IdentifiedDataSerializable Serialization

For a faster serialization of objects, Hazelcast recommends to implement the `IdentifiedDataSerializable` interface. The following is an example of an object implementing this interface:

```C++
class Employee : public serialization::IdentifiedDataSerializable {
public:
    static const int TYPE_ID = 100;

    virtual int getFactoryId() const {
        return 1000;
    }

    virtual int getClassId() const {
        return TYPE_ID;
    }

    virtual void writeData(serialization::ObjectDataOutput &writer) const {
        writer.writeInt(id);
        writer.writeUTF(&name);
    }

    virtual void readData(serialization::ObjectDataInput &reader) {
        id = reader.readInt();
        name = *reader.readUTF();
    }

private:
    int id;
    std::string name;
};
```
The `IdentifiedDataSerializable` interface uses `getClassId()` and `getFactoryId()` to reconstitute the object. To complete the implementation, `DataSerializableFactory` should also be implemented and registered into `SerializationConfig` which can be accessed from `clientConfig.getSerializationConfig().addDataSerializableFactory`. The factory's responsibility is to return an instance of the right `IdentifiedDataSerializable` object, given the `classId`. 

A sample `DataSerializableFactory` could be implemented as follows:

```C++
class SampleDataSerializableFactory : public serialization::DataSerializableFactory {
public:
    static const int FACTORY_ID = 1000;

    virtual std::auto_ptr<serialization::IdentifiedDataSerializable> create(int32_t classId) {
        switch (classId) {
            case 100:
                return std::auto_ptr<serialization::IdentifiedDataSerializable>(new Employee());
            default:
                return std::auto_ptr<serialization::IdentifiedDataSerializable>();
        }

    }
};
```

The last step is to register the `IdentifiedDataSerializableFactory` to the `SerializationConfig`.

```C++
    ClientConfig clientConfig;
    clientConfig.getSerializationConfig().addDataSerializableFactory(SampleDataSerializableFactory::FACTORY_ID,
                                                                     boost::shared_ptr<serialization::DataSerializableFactory>(
                                                                             new SampleDataSerializableFactory()));
```

Note that the ID that is passed to the `SerializationConfig` is same as the `factoryId` that the `Employee` object returns.

## 4.2. Portable Serialization

As an alternative to the existing serialization methods, Hazelcast offers portable serialization. To use it, you need to implement the `Portable` interface. Portable serialization has the following advantages:

- Supporting multiversion of the same object type.
- Fetching individual fields without having to rely on the reflection.
- Querying and indexing support without deserialization and/or reflection.

In order to support these features, a serialized `Portable` object contains meta information like the version and concrete location of the each field in the binary data. This way Hazelcast is able to navigate in the binary data and deserialize only the required field without actually deserializing the whole object which improves the query performance.

With multiversion support, you can have two members where each of them having different versions of the same object, and Hazelcast will store both meta information and use the correct one to serialize and deserialize portable objects depending on the member. This is very helpful when you are doing a rolling upgrade without shutting down the cluster.

Also note that portable serialization is totally language independent and is used as the binary protocol between Hazelcast server and clients.

A sample portable implementation of a `PortableSerializableSample` class looks like the following:

```C++
class PortableSerializableSample : public serialization::Portable {
public:
    static const int CLASS_ID = 1;

    virtual int getFactoryId() const {
        return 1;
    }

    virtual int getClassId() const {
        return CLASS_ID;
    }

    virtual void writePortable(serialization::PortableWriter &writer) const {
        writer.writeInt("id", id);
        writer.writeUTF("name", &name);
        writer.writeLong("lastOrder", lastOrder);
    }

    virtual void readPortable(serialization::PortableReader &reader) {
        id = reader.readInt("id");
        name = *reader.readUTF("name");
        lastOrder = reader.readLong("lastOrder");
    }

private:
    std::string name;
    int32_t id;
    int64_t lastOrder;
};
```

Similar to `IdentifiedDataSerializable`, a `Portable` object must provide `classId` and `factoryId`. The factory object will be used to create the `Portable` object given the `classId`.

A sample `PortableFactory` could be implemented as follows:

```C++
class SamplePortableFactory : public serialization::PortableFactory {
public:
    static const int FACTORY_ID = 1;

    virtual std::auto_ptr<serialization::Portable> create(int32_t classId) const {
        switch (classId) {
            case 1:
                return std::auto_ptr<serialization::Portable>(new PortableSerializableSample());
            default:
                return std::auto_ptr<serialization::Portable>();
        }
    }
};
```

The last step is to register the `PortableFactory` to the `SerializationConfig`.

```C++
    ClientConfig clientConfig;
    clientConfig.getSerializationConfig().addPortableFactory(SamplePortableFactory::FACTORY_ID,
                                                             boost::shared_ptr<serialization::PortableFactory>(
                                                                     new SamplePortableFactory()));
```

Note that the ID that is passed to the `SerializationConfig` is same as the `factoryId` that `PortableSerializableSample` object returns.

## 4.3. Custom Serialization

Hazelcast lets you plug a custom serializer to be used for serialization of objects. Custom serialization lets you use your existing classes without any modification in code for serialization purposes. They will be serialized/deserialized without any code change to already existing classes.

Let's say you have an object `Musician` and you would like to customize the serialization, since you may want to use an external serializer for your object.

```C++
class C++ {
public:
    Musician() {}

    Musician(const std::string &name) : name(name) {}

    virtual ~Musician() {
    }

    const std::string &getName() const {
        return name;
    }

    void setName(const std::string &value) {
        Musician::name = value;
    }

private:
    std::string name;
};
```

Let's say your custom `MusicianSerializer` will serialize `Musician`.

```C++
class MusicianSerializer : public serialization::StreamSerializer {
public:
    virtual int32_t getHazelcastTypeId() const {
        return 10;
    }

    virtual void write(serialization::ObjectDataOutput &out, const void *object) {
        const Musician *csObject = static_cast<const Musician *>(object);
        const std::string &value = csObject->getName();
        int length = (int) value.length();
        std::vector<hazelcast::byte> bytes;
        for (int i = 0; i < length; ++i) {
            bytes.push_back((hazelcast::byte) value[i]);
        }
        out.writeInt((int) length);
        out.write(bytes);
    }

    virtual void *read(serialization::ObjectDataInput &in) {
        int32_t len = in.readInt();
        std::ostringstream value;
        for (int i = 0; i < len; ++i) {
            value << (char) in.readByte();
        }

        return new Musician(value.str());
    }
};
```

Note that the serializer `getHazelcastTypeId` method must return a unique `id` as Hazelcast will use it to lookup the `MusicianSerializer` while it deserializes the object.

You should provide the free function `int getHazelcastTypeId(const MusicianSerializer *);` in the same namespace to which `MusicianSerializer` class belongs. This function should return the same id with its serializer. This id is used to determine which serializer needs to be used for your classes. 

Now the last required step is to register the `MusicianSerializer` to the configuration.

```C++
    ClientConfig clientConfig;
    clientConfig.getSerializationConfig().registerSerializer(
            boost::shared_ptr<serialization::StreamSerializer>(new MusicianSerializer()));

```

From now on, Hazelcast will use `MusicianSerializer` to serialize `Musician` objects.

## 4.4. Global Serialization

The global serializer is identical to custom serializers from the implementation perspective. The global serializer is registered as a fallback serializer to handle all other objects if a serializer cannot be located for them.

By default, global serializer is used if the object is not `IdentifiedDataSerializable` or `Portable` or there is no custom serializer for it.

**Use Cases:**

* Third party serialization frameworks can be integrated using the global serializer.
* For your custom objects, you can implement a single serializer to handle all of them.

A sample global serializer that integrates with a third party serializer is shown below.

```C++
class GlobalSerializer : public serialization::StreamSerializer {
public:
    virtual int32_t getHazelcastTypeId() const {
        return 20;
    }

    virtual void write(serialization::ObjectDataOutput &out, const void *object) {
        // out.write(MyFavoriteSerializer.serialize(object))
    }

    virtual void *read(serialization::ObjectDataInput &in) {
        // return MyFavoriteSerializer.deserialize(in);
        return NULL;
    }
};
```

You should register the global serializer in the configuration.

```C++
    ClientConfig clientConfig;
    clientConfig.getSerializationConfig().setGlobalSerializer(
            boost::shared_ptr<serialization::StreamSerializer>(new GlobalSerializer()));
```

# 5. Setting Up Client Network

All network related configuration of Hazelcast C++ client is performed programmatically via the `ClientNetworkConfig` object. The following is an example configuration.

Here is an example of configuring the network for C++ Client programmatically.

```C++
    ClientConfig clientConfig;
    clientConfig.getNetworkConfig().addAddress(Address("10.1.1.21", 5701));
    clientConfig.getNetworkConfig().addAddress(Address("10.1.1.22", 5703));
    clientConfig.getNetworkConfig().setSmartRouting(true);
    clientConfig.setRedoOperation(true);
    clientConfig.getNetworkConfig().setConnectionTimeout(6000);
    clientConfig.getNetworkConfig().setConnectionAttemptPeriod(5000);
    clientConfig.getNetworkConfig().setConnectionAttemptLimit(5);
```

## 5.1. Providing Member Addresses

Address list is the initial list of cluster addresses which the client will connect to. The client uses this
list to find an alive member. Although it may be enough to give only one address of a member in the cluster
(since all members communicate with each other), it is recommended that you give the addresses for all the members.

```C++
    ClientConfig clientConfig;
    clientConfig.getNetworkConfig().addAddress(Address("10.1.1.21", 5701));
    clientConfig.getNetworkConfig().addAddress(Address("10.1.1.22", 5703));
```

The provided list is shuffled and tried in a random order. If no address is added to the `ClientNetworkConfig`, then `127.0.0.1:5701` is tried by default.

## 5.2. Setting Smart Routing

Smart routing defines whether the client mode is smart or unisocket. See the [C++ Client Operation Modes section](#72-cpp-client-operation-modes)
for the description of smart and unisocket modes.
 
The following is an example configuration.

```C++
    ClientConfig clientConfig;
    clientConfig.getNetworkConfig().setSmartRouting(true);
```

Its default value is `true` (smart client mode).

## 5.3. Enabling Redo Operation

It enables/disables redo-able operations. While sending the requests to the related members, the operations can fail due to various reasons. Read-only operations are retried by default. If you want to enable retry for the other operations, you can set the `redoOperation` to `true`.

```C++
    ClientConfig clientConfig;
    clientConfig.setRedoOperation(true);
```

Its default value is `false` (disabled).

## 5.4. Setting Connection Timeout

Connection timeout is the timeout value in milliseconds for the members to accept the client connection requests.
If the member does not respond within the timeout, the client will retry to connect as many as `ClientNetworkConfig::getConnectionAttemptLimit()` times.
 
The following is an example configuration.

```C++
    ClientConfig clientConfig;
    clientConfig.getNetworkConfig().setConnectionTimeout(6000);
```

Its default value is `5000` milliseconds.

## 5.5. Setting Connection Attempt Limit

While the client is trying to connect initially to one of the members in the `ClientNetworkConfig::getAddresses()` (and the cluster member list addresses if the cluster member list were loaded at least once during a previous connection to the cluster), that member might not be available at that moment. Instead of giving up, throwing an error and stopping the client, the client will retry as many as `ClientNetworkConfig::getConnectionAttemptLimit()` times. This is also the case when the previously established connection between the client and that member goes down.

The following is an example configuration.

```C++
    ClientConfig clientConfig;
    clientConfig.getNetworkConfig().setConnectionAttemptLimit(5);
```

Its default value is `2`.

## 5.6. Setting Connection Attempt Period

Connection attempt period is the duration in milliseconds between the connection attempts defined by `ClientNetworkConfig::getConnectionAttemptPeriod()`.
 
The following is an example configuration.

```C++
    ClientConfig clientConfig;
    clientConfig.getNetworkConfig().setConnectionAttemptPeriod(5000);
```

Its default value is `3000` milliseconds.

## 5.7. Enabling Client TLS/SSL

You can use TLS/SSL to secure the connection between the clients and members. If you want to enable TLS/SSL
for the client-cluster connection, you should set an SSL configuration. Please see [TLS/SSL section](#61-tlsssl).

As explained in the [TLS/SSL section](#61-tlsssl), Hazelcast members have key stores used to identify themselves (to other members) and Hazelcast C++ clients have certificate authorities used to define which members they can trust. 

## 5.8. Enabling Hazelcast AWS Cloud Discovery

The C++ client can discover the existing Hazelcast servers in the Amazon AWS environment. The client queries the Amazon AWS environment using the [describe-instances] (http://docs.aws.amazon.com/cli/latest/reference/ec2/describe-instances.html) query of AWS. The client finds only the up and running instances and filters them based on the filter config provided at the `ClientAwsConfig` configuration.
 
The following is an example configuration:

```C++
clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
            setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(getenv("AWS_SECRET_ACCESS_KEY")).
            setTagKey("aws-test-tag").setTagValue("aws-tag-value-1").setSecurityGroupName("MySecureGroup").setRegion("us-east-1");
```

You need to enable the discovery by calling the `setEnabled(true)`. You can set your access key and secret in the configuration as shown in this example. You can filter the instances by setting which tags they have or by the security group setting. You can set the region for which the instances will be retrieved from, the default region is `us-east-1`.
 
The C++ client works the same way as the Java client. For details, see [AWSClient Configuration] (https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#awsclient-configuration) and [Hazelcast AWS Plugin] (https://github.com/hazelcast/hazelcast-aws/blob/master/README.md). 

# 6. Securing Client Connection

This chapter describes the security features of Hazelcast C++ client. These include using TLS/SSL for connections between members and between clients and members. These security features require **Hazelcast IMDG Enterprise** edition.

### 6.1. TLS/SSL

One of the offers of Hazelcast is the TLS/SSL protocol which you can use to establish an encrypted communication across your cluster with key stores and trust stores.

* A Java `keyStore` is a file that includes a private key and a public certificate.
* A Java `trustStore` is a file that includes a list of certificates trusted by your application which is named as  "certificate authority".

You should set `keyStore` and `trustStore` before starting the members. See the next section on setting `keyStore` and `trustStore` on the server side.

#### 6.1.1. TLS/SSL for Hazelcast Members

Hazelcast allows you to encrypt socket level communication between Hazelcast members and between Hazelcast clients and members, for end to end encryption. To use it, see the [TLS/SSL for Hazelcast Members section](http://docs.hazelcast.org/docs/latest/manual/html-single/index.html#tls-ssl-for-hazelcast-members) in the Hazelcast IMDG Reference Manual.

#### 6.1.2. TLS/SSL for Hazelcast C++ Clients

To use TLS/SSL with your Hazelcast C++ client, you should perform the following:

* Provide the compile flag `-DHZ_BUILD_WITH_SSL` when compiling, since the TLS feature depends on OpenSSL library.
* Install the OpenSSL library to your development environment.
* Enable the SSL in the client network configuration.
* Specify the correct path to the CA verification file for the trusted server. This path can be relative to the executable working directory.
* If needed, set the cipher suite to be used via the SSL configuration.

You can set the protocol type. If not set, the configuration uses `tlsv12` (TLSv1.2) as the default protocol type and version..

The following is an example configuration.

```C++
config.getNetworkConfig().getSSLConfig().setEnabled(true).addVerifyFile(getCAFilePath());

//Cipher suite is set using a string which is defined by OpenSSL standard at this page: https://www.openssl.org/docs/man1.0.2/apps/ciphers.html An example usage:
config.getNetworkConfig().getSSLConfig().setCipherList("HIGH");
```

The detailed config API is below:
```
/**
 * Default protocol is tlsv12 and ssl is disabled by default
 */
SSLConfig();

/**
 * Returns if this configuration is enabled.
 *
 * @return true if enabled, false otherwise
 */
bool isEnabled() const;

/**
 * Enables and disables this configuration.
 *
 * @param enabled true to enable, false to disable
 */
SSLConfig &setEnabled(bool enabled);

/**
 * Sets the ssl protocol to be used for this SSL socket.
 *
 * @param protocol One of the supported protocols
 */
SSLConfig &setProtocol(SSLProtocol protocol);

/**
 * @return The configured SSL protocol
 */
SSLProtocol getProtocol() const;

/**
 * @return The list of all configured certificate verify files for the client.
 */
const std::vector<std::string> &getVerifyFiles() const;

/**
 * This API calls the OpenSSL SSL_CTX_load_verify_locations method underneath while starting the client
 * with this configuration. The validity of the files are checked only when the client starts. Hence,
 * this call will not do any error checking. Error checking is performed only when the certificates are
 * actually loaded during client start.
 *
 * @param filename the name of a file containing certification authority certificates in PEM format.
 */
SSLConfig &addVerifyFile(const std::string &filename);

/**
 * @return Returns the use configured cipher list string.
 */
const std::string &getCipherList() const;

/**
 * @param ciphers The list of ciphers to be used. During client start, if this API was set then the
 * SSL_CTX_set_cipher_list (https://www.openssl.org/docs/man1.0.2/ssl/SSL_set_cipher_list.html) is
 * called with the provided ciphers string. The values and the format of the ciphers are described here:
 * https://www.openssl.org/docs/man1.0.2/apps/ciphers.html Some examples values for the string are:
 * "HIGH", "MEDIUM", "LOW", etc.
 *
 * If non of the provided ciphers could be selected the client initialization will fail.
 *
 */
SSLConfig &setCipherList(const std::string &ciphers);
```

You can set the protocol as one of the following values:
```
sslv2, sslv3, tlsv1, sslv23, tlsv11, tlsv12
```
The `SSLConfig.setEnabled` method should be called explicitly to enable the SSL. The path of the certificate should be correctly provided. 

# 7. Using C++ Client with Hazelcast IMDG

This chapter provides information on how you can use Hazelcast IMDG's data structures in the C++ client, after giving some basic information including an overview to the client API, operation modes of the client and how it handles the failures.

## 7.1. C++ Client API Overview

This chapter provides information on how you can use Hazelcast IMDG's data structures in the C++ client, after giving some basic information including an overview to the client API, operation modes of the client and how it handles the failures.

Most of the methods in C++ API are synchronous. The failures are communicated via exceptions. All exceptions are derived from the `hazelcast::client::exception::IException` base method. There are also asynchronous versions of some methods in the API. The asynchronous ones use the `hazelcast::client::Future<T>` future object. It works similar to the `std::future`.

If you are ready to go, let's start to use Hazelcast C++ client!

The first step is the configuration. You can configure the C++ client programmatically. See the [Programmatic Configuration section](#311-programmatic-configuration) for details. 

The following is an example on how to create a `ClientConfig` object and configure it programmatically:

```C++
    ClientConfig clientConfig;
    clientConfig.getGroupConfig().setName("dev");
    clientConfig.getNetworkConfig().addAddress(Address("'10.90.0.1", 5701)).addAddress(Address("'10.90.0.2", 5702));
```

The second step is initializing the `HazelcastClient` to be connected to the cluster:

```C++
    HazelcastClient client(clientConfig);
    // some operation
```

**This client object is your gateway to access all the Hazelcast distributed objects.**

Let's create a map and populate it with some data, as shown below.

```C++
    // Get the Distributed Map from Cluster.
    IMap<std::string, std::string> map = client.getMap<std::string, std::string>("my-distributed-map");
    //Standard Put and Get.
    map.put("key", "value");
    map.get("key");
    //Concurrent Map methods, optimistic updating
    map.putIfAbsent("somekey", "somevalue");
    map.replace("key", "value", "newvalue");
 ```

As the final step, if you are done with your client, you can shut it down as shown below. This will release all the used resources and close connections to the cluster.

```C++
    // Shutdown this Hazelcast Client
    client.shutdown();
```

The client object destructor also shuts down the client upon destruction if you do not explicitly call the shutdown method.

## 7.2. C++ Client Operation Modes

The client has two operation modes because of the distributed nature of the data and cluster: smart and unisocket.

### 7.2.1. Smart Client

In the smart mode, the clients connect to each cluster member. Since each data partition uses the well known and consistent hashing algorithm, each client can send an operation to the relevant cluster member, which increases the overall throughput and efficiency. Smart mode is the default mode.

### 7.2.2. Unisocket Client

For some cases, the clients can be required to connect to a single member instead of each member in the cluster. Firewalls, security or some custom networking issues can be the reason for these cases.

In the unisocket client mode, the client will only connect to one of the configured addresses. This single member will behave as a gateway to the other members. For any operation requested from the client, it will redirect the request to the relevant member and return the response back to the client returned from this member.

You can set the unisocket client mode in the `ClientConfig` as shown below.

```C++
    clientConfig.getNetworkConfig().setSmartRouting(false);
```

## 7.3. Handling Failures

There are two main failure cases you should be aware of. Below sections explain these and the configurations you can perform to achieve proper behavior.

### 7.3.1. Handling Client Connection Failure

While the client is trying to connect initially to one of the members in the `ClientNetworkConfig::getAddresses()`, all the members might not be available. Instead of giving up, throwing an error and stopping the client, the client will retry as many as `connectionAttemptLimit` times. 

You can configure `connectionAttemptLimit` for the number of times you want the client to retry connecting. See the [Setting Connection Attempt Limit section](#55-setting-connection-attempt-limit).

The client executes each operation through the already established connection to the cluster. If this connection(s) disconnects or drops, the client will try to reconnect as configured.

### 7.3.2. Handling Retry-able Operation Failure

While sending the requests to the related members, the operations can fail due to various reasons. Read-only operations are retried by default. If you want to enable retrying for the other operations, you can set the `redoOperation` to `true`. See the [Enabling Redo Operation section](#53-enabling-redo-operation).

You can set a timeout for retrying the operations sent to a member. This can be provided by using the property `hazelcast.client.invocation.timeout.seconds` in `ClientConfig.properties`. The client will retry an operation within this given period, of course, if it is a read-only operation or you enabled the `redoOperation` as stated in the above paragraph. This timeout value is important when there is a failure resulted by either of the following causes:

* Member throws an exception.
* Connection between the client and member is closed.
* Client’s heartbeat requests are timed out.

When a connection problem occurs, an operation is retried if it is certain that it has not run on the member yet or if it is idempotent such as a read-only operation, i.e., retrying does not have a side effect. If it is not certain whether the operation has run on the member, then the non-idempotent operations are not retried. However, as explained in the first paragraph of this section, you can force all the client operations to be retried (`redoOperation`) when there is a connection failure between the client and member. But in this case, you should know that some operations may run multiple times causing conflicts. For example, assume that your client sent a `queue.offer` operation to the member and then the connection is lost. Since there will be no response for this operation, you will not know whether it has run on the member or not. If you enabled `redoOperation`, it means this operation may run again, which may cause two instances of the same object in the queue.

When invocation is being retried, the client may wait some time before it retries again. This duration can be configured using the following property:

```
config.setProperty(“hazelcast.client.invocation.retry.pause.millis”, “500");
```
The default retry wait time is 1 second.

### 7.3.3. Client Backpressure

Hazelcast uses operations to make remote calls. For example, a `map.get` is an operation and a `map.put` is one operation for the primary
and one operation for each of the backups, i.e., `map.put` is executed for the primary and also for each backup. In most cases, there will be a natural balance between the number of threads performing operations
and the number of operations being executed. However, there are two situations where this balance and operations
can pile up and eventually lead to `OutOfMemoryException` (OOME):

- Asynchronous calls: With async calls, the system may be flooded with the requests.
- Asynchronous backups: The asynchronous backups may be piling up.

To prevent the system from crashing, Hazelcast provides back pressure. Back pressure works by:

- limiting the number of concurrent operation invocations,
- periodically making an async backup sync.

Sometimes, e.g., when your servers are overloaded, you may want to slow down the client operations to the cluster. Then the client can be configured to wait until number of outstanding invocations whose responses are not received to become less than a certain number. This is called Client Back Pressure. By default, the backpressure is disabled. There are a few properties which control the back pressure. The following are these client configuration properties:

- `hazelcast.client.max.concurrent.invocations`: The maximum number of concurrent invocations allowed. To prevent the system from overloading, you can apply a constraint on the number of concurrent invocations. If the maximum number of concurrent invocations has been exceeded and a new invocation comes in, then Hazelcast will throw `HazelcastOverloadException`. By default this property is configured as INT32_MAX.
- `hazelcast.client.invocation.backoff.timeout.millis`: Controls the maximum timeout in milliseconds to wait for an invocation space to be available. If an invocation can't be made because there are too many pending invocations, then an exponential backoff is done to give the system time to deal with the backlog of invocations. This property controls how long an invocation is allowed to wait before getting `HazelcastOverloadException`. When set to -1 then `HazelcastOverloadException` is thrown immediately without any waiting. This is the default value.

For details of backpressure, see the [Back Pressure section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#back-pressure) in the Hazelcast IMDG Reference Manual.

## 7.3.4 Client Connection Strategy

Hazelcast client-cluster connection and reconnection strategy can be configured. Sometimes, you may not want your application to wait for the client to connect to the cluster, you may just want to get the client and let the client connect in the background. This is configured as follows:

```
ClientConfig::getConnectionStrategyConfig().setAsyncStart(bool);
```

When this configuration is set to true, the client creation won't wait to connect to cluster. The client instance will throw an exception for any request, until it connects to the cluster and become ready.

If it is set to false (the default case), `HazelcastClient(const ClientConfig)` will block until a cluster connection is established and it's ready to use the client instance.

### 7.3.4.1 Configuring Client Reconnect Strategy

You can configure how the client should act when the client disconnects from the cluster for any reason. This is configured as follows:

```
ClientConfig::getConnectionStrategyConfig().setReconnectMode(const hazelcast::client::config::ClientConnectionStrategyConfig::ReconnectMode &);
```

Possible values for `ReconnectMode` are:

- `OFF`: Prevents reconnection to the cluster after a disconnect.
- `ON`: Reconnects to the cluster by blocking invocations.
- `ASYNC`: Reconnects to the cluster without blocking invocations. Invocations will receive `HazelcastClientOfflineException`.

## 7.4. Using Distributed Data Structures

Most of the distributed data structures are supported by the C++ client. In this chapter, you will learn how to use these distributed data structures.

### 7.4.1. Using Map

Hazelcast Map (`IMap`) is a distributed map. Through the C++ client, you can perform operations like reading and writing from/to a Hazelcast Map with the well known get and put methods. For details, see the [Map section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#map) in the Hazelcast IMDG Reference Manual.

A Map usage example is shown below.

```C++
    // Get the Distributed Map from Cluster.
    IMap<std::string, std::string> map = hz.getMap<std::string, std::string>("my-distributed-map");
    //Standard Put and Get.
    map.put("key", "value");
    map.get("key");
    //Concurrent Map methods, optimistic updating
    map.putIfAbsent("somekey", "somevalue");
    map.replace("key", "value", "newvalue");
```

### 7.4.2. Using MultiMap

Hazelcast `MultiMap` is a distributed and specialized map where you can store multiple values under a single key. For details, see the [MultiMap section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#multimap) in the Hazelcast IMDG Reference Manual.

A MultiMap usage example is shown below.

```C++
    // Get the Distributed MultiMap from Cluster.
    MultiMap<std::string, std::string> multiMap = hz.getMultiMap<std::string, std::string>("my-distributed-multimap");
    // Put values in the map against the same key
    multiMap.put("my-key", "value1");
    multiMap.put("my-key", "value2");
    multiMap.put("my-key", "value3");
    // Print out all the values for associated with key called "my-key"
    std::vector<std::string> values = multiMap.get("my-key");
    for (std::vector<std::string>::const_iterator it = values.begin();it != values.end(); ++it) {
        std::cout << *it << std::endl; // will print value1, value2 and value3 each per line.
    }
    // remove specific key/value pair
    multiMap.remove("my-key", "value2");    // Shutdown this Hazelcast Client
```

### 7.4.3. Using Replicated Map

Hazelcast `ReplicatedMap` is a distributed key-value data structure where the data is replicated to all members in the cluster. It provides full replication of entries to all members for high speed access. For details, see the [Replicated Map section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#replicated-map) in the Hazelcast IMDG Reference Manual.

A Replicated Map usage example is shown below.

```C++
    boost::shared_ptr<hazelcast::client::ReplicatedMap<int, std::string> > replicatedMap = hz.getReplicatedMap<int, std::string>("myReplicatedMap");
    replicatedMap->put(1, "Furkan");
    replicatedMap->put(2, "Ahmet");
    std::cout << "Replicated map value for key 2 is " << *replicatedMap->get(2) << std::endl; // Replicated map value for key 2 is Ahmet
```

### 7.4.4. Using Queue

Hazelcast Queue (`IQueue`) is a distributed queue which enables all cluster members to interact with it. For details, see the [Queue section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#queue) in the Hazelcast IMDG Reference Manual.

A Queue usage example is shown below.

```C++
    // Get a Blocking Queue called "my-distributed-queue"
    IQueue<std::string> queue = hz.getQueue<std::string>("my-distributed-queue");
    // Offer a String into the Distributed Queue
    queue.offer("item");
    // Poll the Distributed Queue and return the String
    boost::shared_ptr<std::string> item = queue.poll(); // gets first "item"
    //Timed blocking Operations
    queue.offer("anotheritem", 500);
    boost::shared_ptr<std::string> anotherItem = queue.poll(5 * 1000);
    //Indefinitely blocking Operations
    queue.put("yetanotheritem");
    std::cout << *queue.take() << std::endl; // Will print yetanotheritem
```

### 7.4.5. Using Set

Hazelcast Set (`ISet`) is a distributed set which does not allow duplicate elements. For details, see the [Set section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#set) in the Hazelcast IMDG Reference Manual.

A Set usage example is shown below.

```C++
    // Get the Distributed Set from Cluster.
    ISet<std::string> set = hz.getSet<std::string>("my-distributed-set");
    // Add items to the set with duplicates
    set.add("item1");
    set.add("item1");
    set.add("item2");
    set.add("item2");
    set.add("item2");
    set.add("item3");
    // Get the items. Note that there are no duplicates.
    std::vector<std::string> values = set.toArray();
    for (std::vector<std::string>::const_iterator it=values.begin();it != values.end();++it) {
        std::cout << (*it) << std::endl;
    }
```

### 7.4.6. Using List

Hazelcast List (`IList`) is a distributed list which allows duplicate elements and preserves the order of elements. For details, see the [List section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#list) in the Hazelcast IMDG Reference Manual.

A List usage example is shown below.

```C++
    // Get the Distributed List from Cluster.
    IList<std::string> list = hz.getList<std::string>("my-distributed-list");
    // Add elements to the list
    list.add("item1");
    list.add("item2");

    // Remove the first element
    std::cout << "Removed: " << *list.remove(0);
    // There is only one element left
    std::cout << "Current size is " << list.size() << std::endl;
    // Clear the list
    list.clear();
```

### 7.4.7. Using Ringbuffer

Hazelcast `Ringbuffer` is a replicated but not partitioned data structure that stores its data in a ring-like structure. You can think of it as a circular array with a given capacity. Each Ringbuffer has a tail and a head. The tail is where the items are added and the head is where the items are overwritten or expired. You can reach each element in a Ringbuffer using a sequence ID, which is mapped to the elements between the head and tail (inclusive) of the Ringbuffer. For details, see the [Ringbuffer section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#ringbuffer) in the Hazelcast IMDG Reference Manual.

A Ringbuffer usage example is shown below.

```C++
    boost::shared_ptr<Ringbuffer<long> > rb = hz.getRingbuffer<long>("rb");
    // add two items into ring buffer
    rb->add(100);
    rb->add(200);
    // we start from the oldest item.
    // if you want to start from the next item, call rb.tailSequence()+1
    int64_t sequence = rb->headSequence();
    std::cout << *rb->readOne(sequence) << std::endl; //
    sequence++;
    std::cout << *rb->readOne(sequence) << std::endl;
```

### 7.4.8. Using Reliable Topic

Hazelcast `ReliableTopic` is a distributed topic implementation backed up by the `Ringbuffer` data structure. For details, see the [Reliable Topic section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#reliable-topic) in the Hazelcast IMDG Reference Manual.

A Reliable Topic usage example is shown below.

```C++
class MyListener : public hazelcast::client::topic::ReliableMessageListener<std::string> {
public:
    MyListener() : startSequence(-1), lastReceivedSequence(-1) {
    }

    MyListener(int64_t sequence) : startSequence(sequence), lastReceivedSequence(-1) {
    }

    virtual ~MyListener() {
    }

    virtual void onMessage(std::auto_ptr<hazelcast::client::topic::Message<std::string> > message) {

        const std::string *object = message->getMessageObject();
        if (NULL != object) {
            std::cout << "[GenericListener::onMessage] Received message: " << *message->getMessageObject() <<
            " for topic:" << message->getName();
        } else {
            std::cout << "[GenericListener::onMessage] Received message with NULL object for topic:" <<
            message->getName();
        }
    }

    virtual int64_t retrieveInitialSequence() const {
        return startSequence;
    }

    virtual void storeSequence(int64_t sequence) {
        lastReceivedSequence = sequence;
    }

    virtual bool isLossTolerant() const {
        return false;
    }

    virtual bool isTerminal(const hazelcast::client::exception::IException &failure) const {
        return false;
    }
    
private:
    int64_t startSequence;
    int64_t lastReceivedSequence;
};


int main() {
    
    std::string topicName("MyReliableTopic");
    boost::shared_ptr<hazelcast::client::ReliableTopic<std::string> > topic = client.getReliableTopic<std::string>(topicName);
    
    MyListener listener;
    const std::string &listenerId = topic->addMessageListener(listener);

    std::cout << "Registered the listener with listener id:" << listenerId << std::endl;

    std::string message("My first message");
    topic->publish(&message);
    
    if (topic->removeMessageListener(listenerId)) {
        std::cout << "Successfully removed the listener " << listenerId << " for topic " << topicName << std::endl;
    } else {
        std::cerr << "Failed to remove the listener " << listenerId << " for topic " << topicName << std::endl;
    }
    
    return 0;
}
```

### 7.4.9 Using Lock

Hazelcast Lock (`ILock`) is a distributed lock implementation. You can synchronize Hazelcast members and clients using a Lock. For details, see the [Lock section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#lock) in the Hazelcast IMDG Reference Manual.

A Lock usage example is shown below.

```C++
    // Get a distributed lock called "my-distributed-lock"
    ILock lock = hz.getILock("my-distributed-lock");
    // Now create a lock and execute some guarded code.
    lock.lock();
    try {
        //do something here
    } catch (...) {
            lock.unlock();
            throw;
    }
```

### 7.4.10 Using Atomic Long

Hazelcast Atomic Long (`IAtomicLong`) is the distributed long which offers most of the operations such as `get`, `set`, `getAndSet`, `compareAndSet` and `incrementAndGet`. For details, see the [Atomic Long section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#iatomiclong) in the Hazelcast IMDG Reference Manual.

An Atomic Long usage example is shown below.

```C++
    hazelcast::client::IAtomicLong counter = hz.getIAtomicLong("counter");

    for (int i = 0; i < 1000 * 1000; ++i) {
        counter.incrementAndGet();
    }

    std::cout << "Count is:" << counter.get() << std::endl; // Counter is 1000000
```

### 7.4.11 Using Semaphore

Hazelcast Semaphore (`ISemaphore`) is a distributed semaphore implementation. For details, see the [Semaphore section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#isemaphore) in the Hazelcast IMDG Reference Manual.

A Semaphore usage example is shown below.

```C++
    hazelcast::client::ISemaphore semaphore = hz.getISemaphore("semaphore");
    semaphore.init(10);
    semaphore.acquire(5);
    std::cout << "Number of available permits: " << semaphore.availablePermits() << std::endl; // Number of available permits: 5
```

### 7.4.12 Using PN Counter

Hazelcast `PNCounter` (Positive-Negative Counter) is a CRDT positive-negative counter implementation. It is an eventually consistent counter given there is no member failure. For details, see the [PN Counter section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#pn-counter) in the Hazelcast IMDG Reference Manual.

A PN Counter usage example is shown below.

```C++
    boost::shared_ptr<hazelcast::client::crdt::pncounter::PNCounter> pnCounter = hz.getPNCounter("pncounterexample");

    std::cout << "Counter started with value:" << pnCounter->get() << std::endl; // Counter started with value:0

    std::cout << "Counter new value after adding is: " << pnCounter->addAndGet(5) << std::endl; // Counter new value after adding is: 5

    std::cout << "Counter new value before adding is: " << pnCounter->getAndAdd(2) << std::endl; // Counter new value before adding is: 5

    std::cout << "Counter new value is: " << pnCounter->get() << std::endl; // Counter new value is: 7

    std::cout << "Decremented counter by one to: " << pnCounter->decrementAndGet() << std::endl; // Decremented counter by one to: 6
```

### 7.4.13 Using Flake ID Generator

Hazelcast `FlakeIdGenerator` is used to generate cluster-wide unique identifiers. Generated identifiers are long primitive values and are k-ordered (roughly ordered). IDs are in the range from 0 to `2^63-1` (maximum signed long value). For details, see the [FlakeIdGenerator section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#flakeidgenerator) in the Hazelcast IMDG Reference Manual.

A Flake ID Generator usage example is shown below.

```C++
    hazelcast::client::FlakeIdGenerator generator = hz.getFlakeIdGenerator("flakeIdGenerator");
    std::cout << "Id : " << generator.newId() << std::endl; // Id : <some unique number>
```

### 7.4.14. Using Transactions

Hazelcast C++ client provides transactional operations like beginning transactions, committing transactions and retrieving transactional data structures like the `TransactionalMap`, `TransactionalSet`, `TransactionalList`, `TransactionalQueue` and `TransactionalMultiMap`.

You can create a `TransactionContext` object using the C++ client to begin, commit and rollback a transaction. You can obtain transaction-aware instances of queues, maps, sets, lists and multimaps via the `TransactionContext` object, work with them and commit or rollback in one shot. For details, see the [Transactions section](https://docs.hazelcast.org//docs/latest/manual/html-single/index.html#transactions) in the Hazelcast IMDG Reference Manual.

```C++
                // Create a Transaction object and begin the transaction
                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                // Get transactional distributed data structures
                TransactionalMap<std::string, std::string> txnMap = context.getMap<std::string, std::string>("transactional-map");
                TransactionalMultiMap<std::string, std::string> txnMultiMap = context.getMultiMap<std::string, std::string>("transactional-multimap");
                TransactionalQueue<std::string> txnQueue = context.getQueue<std::string>("transactional-queue");
                TransactionalSet<std::string> txnSet = context.getSet<std::string>("transactional-set");

                try {
                    boost::shared_ptr<std::string> obj = txnQueue.poll();
                    //process object
                    txnMap.put( "1", "value1" );
                    txnMultiMap.put("2", "value2_1");
                    txnMultiMap.put("2", "value2_2");
                    txnSet.add( "value" );
                    //do other things

                    // Commit the above changes done in the cluster.
                    context.commitTransaction();
                } catch (...) {
                    context.rollbackTransaction();
                    throw;
                }
```

In a transaction, operations will not be executed immediately. Their changes will be local to the `TransactionContext` until committed. However, they will ensure the changes via locks.

For the above example, when `map.put` is executed, no data will be put in the map but the key will be locked against changes. While committing, operations will be executed, the value will be put to the map and the key will be unlocked.

The isolation level in Hazelcast Transactions is `READ_COMMITTED` on the level of a single partition. If you are in a transaction, you can read the data in your transaction and the data that is already committed. If you are not in a transaction, you can only read the committed data.

## 7.5. Distributed Events

This chapter explains when various events are fired and describes how you can add event listeners on a Hazelcast C++ client. These events can be categorized as cluster and distributed data structure events.

### 7.5.1. Cluster Events

You can add event listeners to a Hazelcast C++ client. You can configure the following listeners to listen to the events on the client side:

* Membership Listener: Notifies when a member joins to/leaves the cluster, or when an attribute is changed in a member.
* Lifecycle Listener: Notifies when the client is starting, started, shutting down and shutdown.

#### 7.5.1.1. Listening for Member Events

You can listen the following types of member events in the `Cluster`.

* `memberAdded`: A new member is added to the cluster.
* `memberRemoved`: An existing member leaves the cluster.
* `memberAttributeChanged`: An attribute of a member is changed. See the [Defining Member Attributes section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#defining-member-attributes) in the Hazelcast IMDG Reference Manual to learn about member attributes.

You can use `Cluster` (`HazelcastClient::getCluster()`) object to register for the membership listeners. There are two types of listeners: `InitialMembershipListener` and `MembershipListener`. The difference is that `InitialMembershipListener` also gets notified when the client connects to the cluster and retrieves the whole membership list. You need to implement one of these two interfaces and register an instance of the listener to the cluster.

The following example demonstrates both initial and regular membership listener registrations.

```C++
        class MyInitialMemberListener : public hazelcast::client::InitialMembershipListener {
        public:
            void init(const hazelcast::client::InitialMembershipEvent &event) {
                std::vector<hazelcast::client::Member> members = event.getMembers();
                std::cout << "The following are the initial members in the cluster:" << std::endl;
                for (std::vector<hazelcast::client::Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    std::cout << it->getAddress() << std::endl;
                }
            }
        
            void memberAdded(const hazelcast::client::MembershipEvent &membershipEvent) {
                std::cout << "[MyInitialMemberListener::memberAdded] New member joined:" <<
                membershipEvent.getMember().getAddress() <<
                std::endl;
            }
        
            void memberRemoved(const hazelcast::client::MembershipEvent &membershipEvent) {
                std::cout << "[MyInitialMemberListener::memberRemoved] Member left:" <<
                membershipEvent.getMember().getAddress() << std::endl;
            }
        
            void memberAttributeChanged(const hazelcast::client::MemberAttributeEvent &memberAttributeEvent) {
                std::cout << "[MyInitialMemberListener::memberAttributeChanged] Member attribute:" <<
                memberAttributeEvent.getKey()
                << " changed. Value:" << memberAttributeEvent.getValue() << " for member:" <<
                memberAttributeEvent.getMember().getAddress() << std::endl;
            }
        };
        
        class MyMemberListener : public hazelcast::client::MembershipListener {
        public:
            void memberAdded(const hazelcast::client::MembershipEvent &membershipEvent) {
                std::cout << "[MyMemberListener::memberAdded] New member joined:" << membershipEvent.getMember().getAddress() <<
                std::endl;
            }
        
            void memberRemoved(const hazelcast::client::MembershipEvent &membershipEvent) {
                std::cout << "[MyMemberListener::memberRemoved] Member left:" <<
                membershipEvent.getMember().getAddress() << std::endl;
            }
        
            void memberAttributeChanged(const hazelcast::client::MemberAttributeEvent &memberAttributeEvent) {
                std::cout << "[MyMemberListener::memberAttributeChanged] Member attribute:" << memberAttributeEvent.getKey()
                << " changed. Value:" << memberAttributeEvent.getValue() << " for member:" <<
                memberAttributeEvent.getMember().getAddress() << std::endl;
            }
        };

     int main() {
        hazelcast::client::ClientConfig config;
        hazelcast::client::HazelcastClient hz(config);
        
        hz.getCluster().addMembershipListener(boost::shared_ptr<hazelcast::client::MembershipListener>(new MyMemberListener()));
        hz.getCluster().addMembershipListener(boost::shared_ptr<hazelcast::client::InitialMembershipListener>(new MyInitialMemberListener()));
        ...
```

The `memberAttributeChanged` has its own type of event named as `MemberAttributeEvent`. When there is an attribute change on the member, this event is fired.

#### 7.5.1.3. Listening for Lifecycle Events

The `LifecycleListener` interface notifies for the following events:

* `starting`: The client is starting.
* `started`: The client has started.
* `shuttingDown`: The client is shutting down.
* `shutdown`: The client’s shutdown has completed.
* `clientConnected`: The client is connected to the cluster.
* `clientDisconnected`: The client is disconnected from the cluster.

The following is an example of the `LifecycleListener` that is added to the `ClientConfig` object and its output.

```C++
class ConnectedListener : public hazelcast::client::LifecycleListener {
public:
    virtual void stateChanged(const hazelcast::client::LifecycleEvent &lifecycleEvent) {
        if (lifecycleEvent.getState() == hazelcast::client::LifecycleEvent::CLIENT_CONNECTED) {
            std::cout << "Client connected to the cluster" << std::endl;
        } else if (lifecycleEvent.getState() == hazelcast::client::LifecycleEvent::CLIENT_DISCONNECTED) {
            std::cout << "Client is disconnected from the cluster" << std::endl;
        }
    }
};

int main() {
    ConnectedListener listener;

    hazelcast::client::ClientConfig config;

    // Add a lifecycle listener so that we can track when the client is connected/disconnected
    config.addListener(&listener);

    hazelcast::client::HazelcastClient hz(config);

    hz.shutdown();
    return 0;
}
```

**Output:**

```
24/10/2018 13:54:06.392 INFO: [0x700006a44000] hz.client_1[dev] [3.10.2-SNAPSHOT] Trying to connect to Address[127.0.0.1:5701] as owner member
24/10/2018 13:54:06.395 INFO: [0x7000069c1000] hz.client_1[dev] [3.10.2-SNAPSHOT] Setting ClientConnection{alive=1, connectionId=1, remoteEndpoint=Address[localhost:5701], lastReadTime=2018-10-24 10:54:06.394000, closedTime=never, connected server version=3.11-SNAPSHOT} as owner with principal uuid: 160e2baf-566b-4cc3-ab25-a1b3e34de320 ownerUuid: 7625eae9-4f8e-4285-82c0-be2f2eee2a50
24/10/2018 13:54:06.395 INFO: [0x7000069c1000] hz.client_1[dev] [3.10.2-SNAPSHOT] Authenticated with server Address[localhost:5701], server version:3.11-SNAPSHOT Local address: Address[127.0.0.1:56007]
24/10/2018 13:54:06.495 INFO: [0x700006cd3000] hz.client_1[dev] [3.10.2-SNAPSHOT] 

Members [2]  {
	Member[localhost]:5702 - 44cf4017-5355-4859-9bf5-fd374d7fc69c
	Member[localhost]:5701 - 7625eae9-4f8e-4285-82c0-be2f2eee2a50
}

24/10/2018 13:54:06.600 INFO: [0x700006a44000] hz.client_1[dev] [3.10.2-SNAPSHOT] LifecycleService::LifecycleEvent CLIENT_CONNECTED
Client connected to the cluster
24/10/2018 13:54:12.269 INFO: [0x7fff97559340] hz.client_1[dev] [3.10.2-SNAPSHOT] LifecycleService::LifecycleEvent SHUTTING_DOWN
24/10/2018 13:54:12.270 INFO: [0x7fff97559340] hz.client_1[dev] [3.10.2-SNAPSHOT] Removed connection to endpoint: Address[localhost:5701], connection: ClientConnection{alive=0, connectionId=1, remoteEndpoint=Address[localhost:5701], lastReadTime=2018-10-24 10:54:06.711000, closedTime=2018-10-24 10:54:12.269000, connected server version=3.11-SNAPSHOT}
Client is disconnected from the cluster
24/10/2018 13:54:12.268 INFO: [0x7fff97559340] hz.client_1[dev] [3.10.2-SNAPSHOT] LifecycleService::LifecycleEvent SHUTDOWN
```

### 7.5.2. Distributed Data Structure Events

You can add event listeners to the distributed data structures.

#### 7.5.2.1. Listening for Map Events

You can listen to map-wide or entry-based events by using the functions in the `EntryListener` interface. To listen to these events, you need to implement the relevant `EntryListener` interface. 

An entry-based event is fired after the operations that affect a specific entry. For example, `IMap::put()`, `IMap::remove()` or `IMap::evict()`. You should use the `EntryListener` type to listen these events. An `EntryEvent` object is passed to the listener callback method.

See the following example.

```C++
class MyEntryListener : public hazelcast::client::EntryListener<int, std::string> {
public:
    virtual void entryAdded(const hazelcast::client::EntryEvent<int, std::string> &event) {
        std::cout << "Entry added:" << event.getKey() << " --> " << event.getValue() << std::endl;
    }

    virtual void entryRemoved(const hazelcast::client::EntryEvent<int, std::string> &event) {
    }

    virtual void entryUpdated(const hazelcast::client::EntryEvent<int, std::string> &event) {
    }

    virtual void entryEvicted(const hazelcast::client::EntryEvent<int, std::string> &event) {
    }

    virtual void entryExpired(const hazelcast::client::EntryEvent<int, std::string> &event) {
    }

    virtual void entryMerged(const hazelcast::client::EntryEvent<int, std::string> &event) {
    }

    virtual void mapEvicted(const hazelcast::client::MapEvent &event) {
    }

    virtual void mapCleared(const hazelcast::client::MapEvent &event) {
    }

};

...
int main() {
    hazelcast::client::ClientConfig config;

    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<int, std::string> map = hz.getMap<int, std::string>("EntryListenerExampleMap");
    MyEntryListener entryListener;
    map.addEntryListener(entryListener, false);
    
    map.put(1, "My new entry");
    
    ...

    return 0;
}
```

A map-wide event is fired as a result of a map-wide operation. For example, `IMap::clear()` or `IMap::evictAll()`. You should use the `EntryListener` type to listen to these events. A `MapEvent` object is passed to the listener method.

See the following example.

```C++
class MyEntryListener : public hazelcast::client::EntryListener<int, std::string> {
public:
    virtual void entryAdded(const hazelcast::client::EntryEvent<int, std::string> &event) {
    }

    virtual void entryRemoved(const hazelcast::client::EntryEvent<int, std::string> &event) {
    }

    virtual void entryUpdated(const hazelcast::client::EntryEvent<int, std::string> &event) {
    }

    virtual void entryEvicted(const hazelcast::client::EntryEvent<int, std::string> &event) {
    }

    virtual void entryExpired(const hazelcast::client::EntryEvent<int, std::string> &event) {
    }

    virtual void entryMerged(const hazelcast::client::EntryEvent<int, std::string> &event) {
    }

    virtual void mapEvicted(const hazelcast::client::MapEvent &event) {
    }

    virtual void mapCleared(const hazelcast::client::MapEvent &event) {
        std::cout << "Map cleared:" << event.getNumberOfEntriesAffected() << std::endl; // Map Cleared: 3
    }

};

int main() {
    hazelcast::client::ClientConfig config;

    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<int, std::string> map = hz.getMap<int, std::string>("EntryListenerExampleMap");
    MyEntryListener entryListener;
    map.addEntryListener(entryListener, false);

    map.put(1, "Mali");
    map.put(2, "Ahmet");
    map.put(3, "Furkan");
    map.clear();

    ...

    return 0;
}
```

## 7.6. Distributed Computing

This section describes how Hazelcast IMDG's distributed executor service and entry processor features can be used in the C++ client.

### 7.6.1. Distributed Executor Service
Hazelcast C++ client allows you to asynchronously execute your tasks (logical units of work) in the cluster, such as database queries, complex calculations and image rendering.

With `IExecutorService`, you can execute tasks asynchronously and perform other useful tasks. If your task execution takes longer than expected, you can cancel the task execution. Tasks should be `Hazelcast Serializable`, i.e., `IdentifiedDataSerializable`, `Portable`, `Custom`, since they will be distributed in the cluster.

You need to implement the actual task logic at the server side as a Java code. The task should implement Java's `java.util.concurrent.Callable` interface.

Note that, the distributed executor service (`IExecutorService`) is intended to run processing where the data is hosted: on the server members.

For more information on the server side configuration, see the [Executor Service section](https://docs.hazelcast.org/docs/latest/manual/html-single/#executor-service) in the Hazelcast IMDG Reference Manual.

#### 7.6.1.1 Implementing a Callable Task

You implement a C++ class which is `Hazelcast Serializable`. This is the task class to be run on the server side. The client side implementation does not need to have any logic, it is purely for initiating the server side task.

On the server side, when you implement the task as `java.util.concurrent.Callable` (a task that returns a value), implement one of the Hazelcast serialization methods for the new class. The serialization type needs to match with that of the client side task class.

An example C++ task class implementation is shown below.

```C++
class MessagePrinter : public serialization::IdentifiedDataSerializable {
public:
    MessagePrinter(const std::string &message) : message(message) {}

    virtual int getFactoryId() const {
        return 1;
    }

    virtual int getClassId() const {
        return 555;
    }

    virtual void writeData(serialization::ObjectDataOutput &writer) const {
        writer.writeUTF(&message);
    }

    virtual void readData(serialization::ObjectDataInput &reader) {
        // no need to implement since it will not be read by the client in our example
    }

private:
    std::string message;
};
```

An example of a Callable Java task which matches the above C++ class is shown below. `MessagePrinter` prints out the message sent from the C++ client at the cluster members.

```Java
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
    public void writeData(ObjectDataOutput out)
            throws IOException {
        out.writeUTF(message);
    }

    @Override
    public void readData(ObjectDataInput in)
            throws IOException {
        message = in.readUTF();
    }

    @Override
    public String call()
            throws Exception {
        System.out.println(message);
        return message;
    }
}
```

You need to compile and link the Java class on the server side (add it to the server's classpath), implement and register a DataSerializableFactory on the server side. In this example, we designed the task as `IdentifiedDataSerializable` and you can see that the factory and class ID for both C++ and Java classes are the same. You can of course use the other Hazelcast serialization methods as well.

#### 7.6.1.2 Executing a Callable Task

To execute a callable task:

* Retrieve the executor from `HazelcastClient`.
* Submit a task which returns a `boost::shared_ptr<ICompletableFuture<T> >`.
* After executing the task, you do not have to wait for the execution to complete, you can process other things.
* When ready, use the `ICompletableFuture<T>` object to retrieve the result as shown in the code example below.

An example where `MessagePrinter` task is executed is shown below.

```C++
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);
    // Get the Distributed Executor Service
    boost::shared_ptr<IExecutorService> ex = hz.getExecutorService("my-distributed-executor");
    // Submit the MessagePrinter Runnable to a random Hazelcast Cluster Member
    boost::shared_ptr<ICompletableFuture<std::string> > future = ex->submit<MessagePrinter, std::string>(MessagePrinter("message to any node"));
    // Wait for the result of the submitted task and print the result
    boost::shared_ptr<std::string> result = future->get();
    std::cout << "Server retuned result: " << *result << std::endl;
```

#### 7.6.1.3 Scaling The Executor Service

You can scale the Executor service both vertically (scale up) and horizontally (scale out). See the [Scaling The Executor Service section](https://docs.hazelcast.org/docs/latest/manual/html-single/#scaling-the-executor-service) in the Hazelcast IMDG Reference Manual for more details on its configuration.

#### 7.6.1.4 Executing Code in the Cluster

The distributed executor service allows you to execute your code in the cluster. In this section, the code examples are based on the `MessagePrinter` class above. The code examples show how Hazelcast can execute your code:

* `printOnTheMember`: On a specific cluster member you choose with the `IExecutorService::submitToMember` method.
* `printOnTheMemberOwningTheKey`: On the member owning the key you choose with the `IExecutorService::submitToKeyOwner` method.
* `printOnSomewhere`: On the member Hazelcast picks with the `IExecutorService::submit` method.
* `printOnMembers`: On all or a subset of the cluster members with the `IExecutorService::submitToMembers` method.

```C++
void printOnTheMember(const std::string &input, const Member &member) {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);

    // Get the Distributed Executor Service
    boost::shared_ptr<IExecutorService> ex = hz.getExecutorService("my-distributed-executor");
    // Submit the MessagePrinter to a the provided Hazelcast Member
    boost::shared_ptr<ICompletableFuture<std::string> > future = ex->submitToMember<MessagePrinter, std::string>(
            MessagePrinter(input), member);

    boost::shared_ptr<std::string> taskResult = future->get();
```

```C++
void printOnTheMemberOwningTheKey(const std::string &input, const std::string &key) {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);

    // Get the Distributed Executor Service
    boost::shared_ptr<IExecutorService> ex = hz.getExecutorService("my-distributed-executor");
    // Submit the MessagePrinter to the cluster member owning the key
    boost::shared_ptr<ICompletableFuture<std::string> > future = ex->submitToKeyOwner<MessagePrinter, std::string, std::string>(
            MessagePrinter(input), key);

    boost::shared_ptr<std::string> taskResult = future->get();
}
```

```C++
void printOnSomewhere(const std::string &input) {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);

    // Get the Distributed Executor Service
    boost::shared_ptr<IExecutorService> ex = hz.getExecutorService("my-distributed-executor");
    // Submit the MessagePrinter to the cluster member owning the key
    boost::shared_ptr<ICompletableFuture<std::string> > future = ex->submit<MessagePrinter, std::string>(
            MessagePrinter(input));

    boost::shared_ptr<std::string> taskResult = future->get();
}
```

```C++
void printOnMembers(const std::string input, const std::vector<Member> &members) {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);

    // Get the Distributed Executor Service
    boost::shared_ptr<IExecutorService> ex = hz.getExecutorService("my-distributed-executor");
    // Submit the MessagePrinter to the cluster member owning the key
    std::map<Member, boost::shared_ptr<ICompletableFuture<std::string> > > futures = ex->submitToMembers<MessagePrinter, std::string>(
            MessagePrinter(input), members);

    for (std::map<Member, boost::shared_ptr<ICompletableFuture<std::string> > >::const_iterator it=futures.begin();it != futures.end(); ++it) {
        boost::shared_ptr<std::string> result = it->second->get();
        std::cout << "Result for member " << it->first << " is " << *result << std::endl;
        // ...
    }
}
```

**NOTE:** You can obtain the set of cluster members via the `HazelcastClient::getCluster()::getMembers()` call.

#### 7.6.1.5 Canceling an Executing Task
A task in the code that you execute in a cluster might take longer than expected. If you cannot stop/cancel that task, it will keep eating your resources.

To cancel a task, you can use the `ICompletableFuture<T>::cancel()` API. This API encourages us to code and design for cancellations, a highly ignored part of software development.

#### 7.6.1.5.1 Example Task to Cancel
The following code waits for the task to be completed in 3 seconds. If it is not finished within this period, a `TimeoutException` is thrown from the `get()` method, and we cancel the task with the `cancel()` method. The remote execution of the task is being cancelled.

```
    try {
        future->get(3, TimeUnit::SECONDS());
    } catch (exception::TimeoutException &e) {
        future->cancel(true);
    }
```

#### 7.6.1.6 Callback When Task Completes

You can use the `ExecutionCallback` interface offered by Hazelcast to asynchronously be notified when the execution is done.

* To be notified when your task completes without an error, implement the `onResponse` method.
* To be notified when your task completes with an error, implement the `onFailure` method.

#### 7.6.1.6.1 Example Task to Callback

The example code below submits the `MessagePrinter` task to `PrinterCallback` and prints the result asynchronously. `ExecutionCallback` has the methods `onResponse` and `onFailure`. In this example code, `onResponse` is called upon a valid response and prints the calculation result, whereas `onFailure` is called upon a failure and prints the exception details.

```C++
    boost::shared_ptr<ExecutionCallback<std::string> > callback(new PrinterCallback());
    ex->submit<MessagePrinter, std::string>(MessagePrinter(input), callback);
```

The `PrinterCallback` implementation class is as below:

```C++
class PrinterCallback : public ExecutionCallback<std::string> {
public:
    virtual void onResponse(const boost::shared_ptr<std::string> &response) {
        std::cout << "The execution of the task is completed successfully and server returned:" << *response << std::endl;
    }

    virtual void onFailure(const boost::shared_ptr<exception::IException> &e) {
        std::cout << "The execution of the task failed with exception:" << e << std::endl;
    }
};
```

#### 7.6.1.7 Selecting Members for Task Execution
As previously mentioned, it is possible to indicate where in the Hazelcast cluster the task is executed. Usually you execute these in the cluster based on the location of a key or set of keys, or you allow Hazelcast to select a member.

If you want more control over where your code runs, use the `MemberSelector` interface. For example, you may want certain tasks to run only on certain members, or you may wish to implement some form of custom load balancing regime.  The `MemberSelector` is an interface that you can implement and then provide to the `IExecutorService` when you submit or execute.

The `bool select(const Member &member)` method is called for every available member in the cluster. Implement this method to decide if the member is going to be used or not.

In the simple example shown below, we select the cluster members based on the presence of an attribute.

```C++
class MyMemberSelector : public hazelcast::client::cluster::memberselector::MemberSelector {
public:
    virtual bool select(const Member &member) const {
        const std::string *attribute = member.getAttribute("my.special.executor");
        if (attribute == NULL) {
            return false;
        }

        return *attribute == "true";
    }

    virtual void toString(std::ostream &os) const {
        os << "MyMemberSelector";
    }
};
```

You can now submit your task using this selector and the task will run on the member whose attribute key "my.special.executor" is set to "true". An example is shown below:

```
ex->submit<MessagePrinter, std::string>(MessagePrinter(input), MyMemberSelector());
```

### 7.6.2. Using EntryProcessor

Hazelcast supports entry processing. An entry processor is a function that executes your code on a map entry in an atomic way.

An entry processor is a good option if you perform bulk processing on an `IMap`. Usually you perform a loop of keys -- executing `IMap.get(key)`, mutating the value and finally putting the entry back in the map using `IMap.put(key,value)`. If you perform this process from a client or from a member where the keys do not exist, you effectively perform two network hops for each update: the first to retrieve the data and the second to update the mutated value.

If you are doing the process described above, you should consider using entry processors. An entry processor executes a read and updates upon the member where the data resides. This eliminates the costly network hops described above.

> **NOTE: Entry processor is meant to process a single entry per call. Processing multiple entries and data structures in an entry processor is not supported as it may result in deadlocks on the server side.**

Hazelcast sends the entry processor to each cluster member and these members apply it to the map entries. Therefore, if you add more members, your processing completes faster.

#### Processing Entries

The `IMap` interface provides the following functions for entry processing:

* `executeOnKey` processes an entry mapped by a key.
* `executeOnKeys` processes entries mapped by a list of keys.
* `executeOnEntries` can process all entries in a map with a defined predicate. Predicate is optional.

In the C++ client, an `EntryProcessor` should be `IdentifiedDataSerializable`, `Portable` or custom serializable, because the server should be able to deserialize it to process.

The following is an example for `EntryProcessor` which is `IdentifiedDataSerializable`.

```C++
class ExampleEntryProcessor : public hazelcast::client::serialization::IdentifiedDataSerializable {
public:
    ExampleEntryProcessor() {}

    ExampleEntryProcessor(const std::string &value) : value(value) {}

    int getFactoryId() const {
        return 5;
    }

    int getClassId() const {
        return 1;
    }

    void writeData(hazelcast::client::serialization::ObjectDataOutput &writer) const {
        writer.writeUTF(&value);
    }

    void readData(hazelcast::client::serialization::ObjectDataInput &reader) {
        value = *reader.readUTF();
    }
    
private:
    std::string value;
};
```

Now, you need to make sure that the Hazelcast member recognizes the entry processor. For this, you need to implement the Java equivalent of your entry processor and its factory, and create your own compiled class or JAR files. For adding your own compiled class or JAR files to the server's `CLASSPATH`, see the [Adding User Library to CLASSPATH section](#1212-adding-user-library-to-classpath).

The following is the Java equivalent of the entry processor in C++ client given above:

```java
import com.hazelcast.map.AbstractEntryProcessor;
import com.hazelcast.nio.ObjectDataInput;
import com.hazelcast.nio.ObjectDataOutput;
import com.hazelcast.nio.serialization.IdentifiedDataSerializable;
import java.io.IOException;
import java.util.Map;

public class IdentifiedEntryProcessor extends AbstractEntryProcessor<String, String> implements IdentifiedDataSerializable {
     static final int CLASS_ID = 1;
     private String value;
     
    public IdentifiedEntryProcessor() {
    }
    
     @Override
    public int getFactoryId() {
        return IdentifiedFactory.FACTORY_ID;
    }
    
     @Override
    public int getId() {
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
    public Object process(Map.Entry<String, String> entry) {
        entry.setValue(value);
        return value;
    }
}
```

You can implement the above processor’s factory as follows:

```java
import com.hazelcast.nio.serialization.DataSerializableFactory;
import com.hazelcast.nio.serialization.IdentifiedDataSerializable;

public class IdentifiedFactory implements DataSerializableFactory {
    public static final int FACTORY_ID = 5;
    
     @Override
    public IdentifiedDataSerializable create(int typeId) {
        if (typeId == IdentifiedEntryProcessor.CLASS_ID) {
            return new IdentifiedEntryProcessor();
        }
        return null;
    }
}
```

Now you need to configure the `hazelcast.xml` to add your factory as shown below.

```xml
<hazelcast>
    <serialization>
        <data-serializable-factories>
            <data-serializable-factory factory-id="5">
                IdentifiedFactory
            </data-serializable-factory>
        </data-serializable-factories>
    </serialization>
</hazelcast>
```

The code that runs on the entries is implemented in Java on the server side. The client side entry processor is used to specify which entry processor should be called. For more details about the Java implementation of the entry processor, see the [Entry Processor section](https://docs.hazelcast.org/docs/latest/manual/html-single/index.html#entry-processor) in the Hazelcast IMDG Reference Manual.

After the above implementations and configuration are done and you start the server where your library is added to its `CLASSPATH`, you can use the entry processor in the `IMap` functions. See the following example.

```C++
    hazelcast::client::IMap<std::string, std::string> map = hz.getMap<std::string, std::string>("my-distributed-map");
    map.executeOnKey<std::string, ExampleEntryProcessor>("key", ExampleEntryProcessor("my new name"));
    std::cout << "Value for 'key' is : '" << *map.get("key") << "'" << std::endl; // value for 'key' is 'my new name'
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

There are many built-in `Predicate` implementations for your query requirements. Some of them are explained below.

* `TruePredicate`: This predicate returns true and hence includes all the entries on the response.
* `FalsePredicate`: This predicate returns false and hence filters out all the entries in the response.
* `EqualPredicate`: Checks if the result of an expression is equal to a given value.
* `NotEqualPredicate`: Checks if the result of an expression is not equal to a given value.
* `InstanceOfPredicate`: Checks if the result of an expression has a certain type.
* `LikePredicate`: Checks if the result of an expression matches some string pattern. `%` (percentage sign) is the placeholder for many characters, `_` (underscore) is placeholder for only one character.
* `ILikePredicate`: Same as LikePredicate. Checks if the result of an expression matches some string pattern. `%` (percentage sign) is the placeholder for many characters, `_` (underscore) is placeholder for only one character.
* `GreaterLessPredicate`: Checks if the result of an expression is greater equal or less equal than a certain value.
* `BetweenPredicate`: Checks if the result of an expression is between two values (start and end values are inclusive).
* `InPredicate`: Checks if the result of an expression is an element of a certain list.
* `NotPredicate`: Negates a provided predicate result.
* `RegexPredicate`: Checks if the result of an expression matches some regular expression.
* `SqlPredicate`: Query using SQL syntax.
* `PagingPredicate`: Returns matching entries page by page.

Hazelcast offers the following ways for distributed query purposes:

* Combining predicates with `AndPredicate`, `OrPredicate` and `NotPredicate`
* Distributed SQL Query

#### 7.7.1.1. Employee Map Query Example

Assume that you have an `employee` map containing the values of `Employee` objects, as coded below. 

```C++
class Employee : public serialization::Portable {
public:
    static const int TYPE_ID = 100;

    Employee() : active(false), salary(0.0) {}

    Employee(int32_t id, const std::string &name, bool active, double salary) : id(id), name(name), active(active),
                                                                            salary(salary) {}

    virtual int getFactoryId() const {
        return 1000;
    }

    virtual int getClassId() const {
        return TYPE_ID;
    }

    virtual void writePortable(serialization::PortableWriter &writer) const {
        writer.writeInt(id);
        writer.writeUTF(&name);
        writer.writeBoolean(active);
        writer.writeDouble(salary);
    }

    virtual void readPortable(serialization::PortableReader &reader) {
        id = reader.readInt();
        name = *reader.readUTF();
        active = reader.readBoolean();
        salary = reader.readDouble();
    }

private:
    int32_t id;
    std::string name;
    bool active;
    double salary;
};
```

Note that `Employee` is implementing `Portable`. As portable types are not deserialized on the server side for querying, you don't need to implement its Java equivalent on the server side for querying.

For the non-portable types, you need to implement its Java equivalent and its serializable factory on the server side for server to reconstitute the objects from binary formats. 
In this case before starting the server, you need to compile the `Employee` and related factory classes with server's `CLASSPATH` and add them to the `user-lib` directory in the extracted `hazelcast-<version>.zip` (or `tar`). See the [Adding User Library to CLASSPATH section](#1212-adding-user-library-to-classpath).

> **NOTE: Querying with `Portable` object is faster as compared to `IdentifiedDataSerializable`.**

#### 7.7.1.2. Querying by Combining Predicates with AND, OR, NOT

You can combine predicates by using the `AndPredicate`, `OrPredicate` and `NotPredicate` operators, as shown in the below example.

```C++
    query::AndPredicate andPredicate;
    andPredicate.add(std::auto_ptr<query::Predicate>(new query::EqualPredicate<bool>("active", true))).add(std::auto_ptr<query::Predicate>(new query::GreaterLessPredicate<int>("age", 30, false, true)));
    std::vector<Employee> activeEmployeesLessThan30 = employees.values(andPredicate);
```

In the above example code, `predicate` verifies whether the entry is active and its `age` value is less than 30. This `predicate` is applied to the `employee` map using the `IMap::values(const query::Predicate &predicate)` method. This method sends the predicate to all cluster members and merges the results coming from them. 

> **NOTE: Predicates can also be applied to `keySet` and `entrySet` of the Hazelcast IMDG's distributed map.**

#### 7.7.1.3. Querying with SQL

`SqlPredicate` takes the regular SQL `where` clause. See the following example:

```C++
    query::SqlPredicate sqlPredicate("active AND age < 30");
    std::vector<Employee> activeEmployeesLessThan30 = employees.values(sqlPredicate);
```

##### Supported SQL Syntax

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

##### Querying Examples with Predicates

You can use the `query::QueryConstants::getKeyAttributeName()` (`__key`) attribute to perform a predicated search for entry keys. See the following example:

```C++
    hazelcast::client::IMap<std::string, int> map = hz.getMap<std::string, int>("personMap;");
    map.put("Mali", 28);
    map.put("Ahmet", 30);
    map.put("Furkan", 23);
    query::SqlPredicate sqlPredicate("__key like F%");
    std::vector<std::pair<std::string, int> > startingWithF = map.entrySet(sqlPredicate);
    std::cout << "First person:" << startingWithF[0].first << ", age:" << startingWithF[0].second << std::endl;
```

In this example, the code creates a list with the values whose keys start with the letter "F”.

You can use `query::QueryConstants::getValueAttributeName()` (`this`) attribute to perform a predicated search for entry values. See the following example:

```C++
    hazelcast::client::IMap<std::string, int> map = hz.getMap<std::string, int>("personMap;");
    map.put("Mali", 28);
    map.put("Ahmet", 30);
    map.put("Furkan", 23);
    query::GreaterLessPredicate<int> greaterThan27Predicate(query::QueryConstants::getValueAttributeName(), 27, false, false);
    std::vector<std::string> olderThan27 = map.keySet(greaterThan27Predicate);
    std::cout << "First person:" << olderThan27[0] << ", second person:" << olderThan27[1] << std::endl;
```

In this example, the code creates a list with the values greater than or equal to "27".

#### 7.7.1.4. Filtering with Paging Predicates

The C++ client provides paging for defined predicates. With its `PagingPredicate` object, you can get a list of keys, values, or entries page by page by filtering them with predicates and giving the size of the pages. Also, you can sort the entries by specifying comparators.

```C++
    hazelcast::client::IMap<std::string, int> map = hz.getMap<std::string, int>("personMap;");
    query::PagingPredicate<std::string, int> pagingPredicate(std::auto_ptr<query::Predicate>(new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 18, false, false)), 5);
    // Set page to retrieve third page
    pagingPredicate.setPage(3);
    std::vector<int> values = map.values(pagingPredicate);
    //...

    // Set up next page
    pagingPredicate.nextPage();
    
    // Retrieve next page    
    values = map.values(pagingPredicate);
    
    //...
```

If you want to sort the result before paging, you need to specify a comparator object that implements the `query::EntryComparator` interface. Also, this comparator object should be one of `IdentifiedDataSerializable` or `Portable` or `Custom serializable`. After implementing After implementing this object in C++, you need to implement the Java equivalent of it and its factory. The Java equivalent of the comparator should implement `java.util.Comparator`. Note that the `compare` function of `Comparator` on the Java side is the equivalent of the `sort` function of `Comparator` on the C++ side. When you implement the `Comparator` and its factory, you can add them to the `CLASSPATH` of the server side.  See the [Adding User Library to CLASSPATH section](#1212-adding-user-library-to-classpath). 

Also, you can access a specific page more easily with the help of the `setPage` function. This way, if you make a query for the 100th page, for example, it will get all 100 pages at once instead of reaching the 100th page one by one using the `nextPage` function.

## 7.8. Performance

### 7.8.1. Partition Aware

Partition Aware ensures that the related entries exist on the same member. If the related data is on the same member, operations can be executed without the cost of extra network calls and extra wire data, and this improves the performance. This feature is provided by using the same partition keys for related data.

Hazelcast has a standard way of finding out which member owns/manages each key object. The following operations are routed to the same member, since all of them are operating based on the same key `'key1'`.

```C++
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hazelcastInstance(config);

    hazelcast::client::IMap<int, int> mapA = hazelcastInstance.getMap<int, int>("mapA");
    hazelcast::client::IMap<int, int> mapB = hazelcastInstance.getMap<int, int>("mapB");
    hazelcast::client::IMap<int, int> mapC = hazelcastInstance.getMap<int, int>("mapC");

    // since map names are different, operation will be manipulating
    // different entries, but the operation will take place on the
    // same member since the keys ("key1") are the same
    mapA.put("key1", value);
    mapB.get("key1");
    mapC.remove("key1");
```

When the keys are the same, entries are stored on the same member. However, we sometimes want to have the related entries stored on the same member, such as a customer and his/her order entries. We would have a customers map with `customerId` as the key and an orders map with `orderId` as the key. Since `customerId` and `orderId` are different keys, a customer and his/her orders may fall into different members in your cluster. So how can we have them stored on the same member? We create an affinity between the customer and orders. If we make them part of the same partition then these entries will be co-located. We achieve this by making `orderKey` s `PartitionAware`.

```C++
class OrderKey : public hazelcast::client::PartitionAware<int64_t>,
          public hazelcast::client::serialization::IdentifiedDataSerializable {
public:
    static const std::string desiredPartitionString;

    OrderKey() {
    }

    OrderKey(int64_t orderId, int64_t customerId) : orderId(orderId), customerId(customerId) {}

    int64_t getOrderId() const {
        return orderId;
    }

    virtual const int64_t *getPartitionKey() const {
        return &customerId;
    }

    virtual int getFactoryId() const {
        return 1;
    }

    virtual int getClassId() const {
        return 10;
    }

    virtual void writeData(hazelcast::client::serialization::ObjectDataOutput &writer) const {
        writer.writeLong(orderId);
        writer.writeLong(customerId);
    }

    virtual void readData(hazelcast::client::serialization::ObjectDataInput &reader) {
        orderId = reader.readLong();
        customerId = reader.readLong();
    }

private:
    int64_t orderId;
    int64_t customerId;
};
```

Notice that `OrderKey` implements `PartitionAware` interface and that `getPartitionKey()` returns the `customerId`. This will make sure that the `Customer` entry and its `Order`s will be stored on the same member.

```C++
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hazelcastInstance(config);

    hazelcast::client::IMap<int64_t, Customer> mapCustomers = hazelcastInstance.getMap<int64_t, Customer>( "customers" );
    hazelcast::client::IMap<OrderKey, Order> mapOrders = hazelcastInstance.getMap<OrderKey, Order>( "orders" );

    // create the customer entry with customer id = 1
    mapCustomers.put( 1, customer );

    // now create the orders for this customer
    mapOrders.put( new OrderKey( 21, 1 ), order );
    mapOrders.put( new OrderKey( 22, 1 ), order );
    mapOrders.put( new OrderKey( 23, 1 ), order );
```  

For more details, see the [PartitionAware section](https://docs.hazelcast.org/docs/latest/manual/html-single/#partitionaware) in the Hazelcast IMDG Reference Manual.

### 7.8.2. Near Cache

Map  entries in Hazelcast are partitioned across the cluster members. Hazelcast clients do not have local data at all. Suppose you read the key `k` a number of times from a Hazelcast client or `k` is owned by another member in your cluster. Then each `map.get(k)` will be a remote operation, which creates a lot of network trips. If you have a data structure that is mostly read, then you should consider creating a local Near Cache, so that reads are sped up and less network traffic is created.

These benefits do not come for free, please consider the following trade-offs:

- Clients with a Near Cache will have to hold the extra cached data, which increases memory consumption.

- If invalidation is enabled and entries are updated frequently, then invalidations will be costly.

- Near Cache breaks the strong consistency guarantees; you might be reading stale data.

Near Cache is highly recommended for maps that are mostly read.

In a client/server system you must enable the Near Cache separately on the client, without the need to configure it on the server. Please note that the Near Cache configuration is specific to the server or client itself: a data structure on a server may not have a Near Cache configured while the same data structure on a client may have it configured. They also can have different Near Cache configurations.

If you are using the Near Cache, you should take into account that your hits to the keys in the Near Cache are not reflected as hits to the original keys on the primary members. This has for example an impact on IMap's maximum idle seconds or time-to-live seconds expiration. Therefore, even though there is a hit on a key in the Near Cache, your original key on the primary member may expire.

NOTE: Near Cache works only when you access data via the `map.get(k)` method. Data returned using a predicate is not stored in the Near Cache.

A Near Cache can have its own `in-memory-format` which is independent of the `in-memory-format` of the data structure.

#### 7.8.2.1 Configuring Near Cache

Hazelcast Map can be configured to work with near cache enabled. You can enable the Near Cache on a Hazelcast Map by adding its configuration for that map. An example configuration for `myMap` is shown below.

```C++
    ClientConfig config;
    const char *mapName = "myMap";
    boost::shared_ptr<config::NearCacheConfig<int, std::string> > nearCacheConfig(
            new config::NearCacheConfig<int, std::string>(mapName, config::OBJECT));
    nearCacheConfig->setInvalidateOnChange(true);
    nearCacheConfig->getEvictionConfig()->setEvictionPolicy(config::LRU)
            .setMaximumSizePolicy(config::EvictionConfig<int, std::string>::ENTRY_COUNT).setSize(100);
    nearCacheConfig->setTimeToLiveSeconds(1);
    nearCacheConfig->setMaxIdleSeconds(2);
    config.addNearCacheConfig(nearCacheConfig);
```

Following are the descriptions of all configuration elements:
 - `setInMemoryFormat`: Specifies in which format data will be stored in your Near Cache. Note that a map’s in-memory format can be different from that of its Near Cache. Available values are as follows:
  - `BINARY`: Data will be stored in serialized binary format (default value).
  - `OBJECT`: Data will be stored in deserialized form.
 - `setInvalidateOnChange`: Specifies whether the cached entries are evicted when the entries are updated or removed in members. Its default value is true.
 - `setTimeToLiveSeconds`: Maximum number of seconds for each entry to stay in the Near Cache. Entries that are older than this period are automatically evicted from the Near Cache. Regardless of the eviction policy used, `timeToLiveSeconds` still applies. Any integer between 0 and `INT32_MAX`. 0 means infinite. Its default value is 0.
 - `setMaxIdleSeconds`: Maximum number of seconds each entry can stay in the Near Cache as untouched (not read). Entries that are not read more than this period are removed from the Near Cache. Any integer between 0 and `INT32_MAX`. 0 means infinite. Its default value is 0. 
 - `setEvictionConfig`: Eviction policy configuration. The following can be set on this config:
    - `setEviction`: Specifies the eviction behavior when you use High-Density Memory Store for your Near Cache. It has the following attributes:
        - `setEvictionPolicy`: Eviction policy configuration. Available values are as follows:
            - `LRU`: Least Recently Used (default value).
            - `LFU`: Least Frequently Used.
            - `NONE`: No items will be evicted and the property max-size will be ignored. You still can combine it with `time-to-live-seconds` and `max-idle-seconds` to evict items from the Near Cache.
            - `RANDOM`: A random item will be evicted.
        - `setMaxSizePolicy`: Maximum size policy for eviction of the Near Cache. Available values are as follows:
            - `ENTRY_COUNT`: Maximum size based on the entry count in the Near Cache (default value).
        - `setSize`: Maximum size of the Near Cache used for `max-size-policy`. When this is reached the Near Cache is evicted based on the policy defined. Any integer between `1` and `INT32_MAX`. Default is `INT32_MAX`.
- `setLocalUpdatePolicy`: Specifies the update policy of the local Near Cache. It is available on JCache clients. Available values are as follows:
    - `INVALIDATE`: Removes the Near Cache entry on mutation. After the mutative call to the member completes but before the operation returns to the caller, the Near Cache entry is removed. Until the mutative operation completes, the readers still continue to read the old value. But as soon as the update completes the Near Cache entry is removed. Any threads reading the key after this point will have a Near Cache miss and call through to the member, obtaining the new entry. This setting provides read-your-writes consistency. This is the default setting.
    - `CACHE_ON_UPDATE`: Updates the Near Cache entry on mutation. After the mutative call to the member completes but before the put returns to the caller, the Near Cache entry is updated. So a remove will remove it and one of the put methods will update it to the new value. Until the update/remove operation completes, the entry's old value can still be read from the Near Cache. But before the call completes the Near Cache entry is updated. Any threads reading the key after this point will read the new entry. If the mutative operation was a remove, the key will no longer exist in the cache, both the Near Cache and the original copy in the member. The member will initiate an invalidate event to any other Near Caches, however the caller Near Cache is not invalidated as it already has the new value. This setting also provides read-your-writes consistency.

#### 7.8.2.2. Near Cache Example for Map
The following is an example configuration for a Near Cache defined in the `mostlyReadMap` map. According to this configuration, the entries are stored as `OBJECT`'s in this Near Cache and eviction starts when the count of entries reaches `5000`; entries are evicted based on the `LRU` (Least Recently Used) policy. In addition, when an entry is updated or removed on the member side, it is eventually evicted on the client side.
```C++
    boost::shared_ptr<config::NearCacheConfig<int, std::string> > nearCacheConfig(
            new config::NearCacheConfig<int, std::string>("mostlyReadMap", config::OBJECT));
    nearCacheConfig->setInvalidateOnChange(true);
    nearCacheConfig->getEvictionConfig()->setEvictionPolicy(config::LRU).setSize(5000);
    config.addNearCacheConfig(nearCacheConfig);
```

#### 7.8.2.3. Near Cache Eviction
In the scope of Near Cache, eviction means evicting (clearing) the entries selected according to the given `evictionPolicy` when the specified `size` has been reached.

The `EvictionConfig::setSize` defines the entry count when the Near Cache is full and determines whether the eviction should be triggered. 

Once the eviction is triggered the configured `evictionPolicy` determines which, if any, entries must be evicted.
 
#### 7.8.2.4. Near Cache Expiration
Expiration means the eviction of expired records. A record is expired:

- if it is not touched (accessed/read) for `maxIdleSeconds`
- `timeToLiveSeconds` passed since it is put to Near Cache

The actual expiration is performed when a record is accessed: it is checked if the record is expired or not. If it is expired, it is evicted and the value shall be looked up from the cluster.

#### 7.8.2.5. Near Cache Invalidation

Invalidation is the process of removing an entry from the Near Cache when its value is updated or it is removed from the original map (to prevent stale reads). See the [Near Cache Invalidation section](https://docs.hazelcast.org/docs/latest/manual/html-single/#near-cache-invalidation) in the Hazelcast IMDG Reference Manual.

## 7.9. Monitoring and Logging

### 7.9.1. Enabling Client Statistics

You can enable the client statistics before starting your clients. There are two properties related to client statistics:

- `hazelcast.client.statistics.enabled`: If set to `true`, it enables collecting the client statistics and sending them to the cluster. When it is `true` you can monitor the clients that are connected to your Hazelcast cluster, using Hazelcast Management Center. Its default value is `false`.

- `hazelcast.client.statistics.period.seconds`: Period in seconds the client statistics are collected and sent to the cluster. Its default value is `3`.

You can enable client statistics and set a non-default period in seconds as follows:

```C++
    hazelcast::client::ClientConfig config;
    config.setProperty(hazelcast::client::ClientProperties::STATISTICS_ENABLED, "true");
    config.setProperty(hazelcast::client::ClientProperties::STATISTICS_PERIOD_SECONDS, "4");
```

After enabling the client statistics, you can monitor your clients using Hazelcast Management Center. Please refer to the [Monitoring Clients section](https://docs.hazelcast.org/docs/management-center/latest/manual/html/index.html#monitoring-clients) in the Hazelcast Management Center Reference Manual for more information on the client statistics.

### 7.9.2. Logging Configuration

By default, the Hazelcast logger prints out the INFO level and above logs to the standard output. The following log levels exist:

* FINEST (DEBUG)
* INFO
* WARNING
* SEVERE (FATAL)

The order is in increasing order. Hence, for INFO level configuration the INFO, WARNING and SEVERE logs are written.

In some applications you may want to use your custom logging statements, you may want to direct the logs to a file instead of standard output, and enable/disable certain log levels. This can be done using the LoggerConfig in the ClientConfig object which is used to configure the client. Each client may have a separate configuration. The customized options needs to be configured using a configuration file. E.g.:

```
clientConfig.getLoggerConfig().setConfigurationFileName("logger-config.txt");
```

The file name is relative path to the application working directory or should be an absolute path. The configuration file will use the format as supported by the configured logger type. Currently, only the easylogging++ (https://github.com/muflihun/easyloggingpp/tree/v8.91) logger is supported, hence the configuration should be done in accordance with the easylogging++ configuration: https://github.com/muflihun/easyloggingpp/tree/v8.91#configuration-file 

If you provide a non-existent or invalid logger configuration file, the library will fail fast by throwing exception::IllegalStateException with the cause of the problem.

> **IMPORTANT NOTE: If you configured the logger configuration file, then the `ClientConfig::setLogLevel` will not be effective since the levels will be controlled from the configuration file.**

## 7.10. Raw Pointer API

When using C++ client you can have the ownership of raw pointers for the objects you create and return. This allows you to keep the objects in your library/application without any need for copy.

For each container, you can use the adapter classes, whose names start with `RawPointer`, to access the raw pointers of the created objects. These adapter classes are found in the `hazelcast::client::adaptor` namespace and listed below:

- `RawPointerList`
- `RawPointerQueue`
- `RawPointerTransactionalMultiMap`
- `RawPointerMap`
- `RawPointerSet`
- `RawPointerTransactionalQueue`
- `RawPointerMultiMap`
- `RawPointerTransactionalMap`

These are adapter classes and they do not create new structures. You just provide the legacy containers as parameters and then you can work with these raw capability containers freely. An example usage of `RawPointerMap` is shown below:

```
hazelcast::client::IMap<std::string, std::string> m = hz.getMap<std::string, std::string>("map");
hazelcast::client::adaptor::RawPointerMap<std::string, std::string> map(m);
map.put("1", "Tokyo");
map.put("2", "Paris");
map.put("3", "New York");
std::cout << "Finished loading map" << std::endl;

std::auto_ptr<hazelcast::client::DataArray<std::string> > vals = map.values();
std::auto_ptr<hazelcast::client::EntryArray<std::string, std::string> > entries = map.entrySet();

std::cout << "There are " << vals->size() << " values in the map" << std::endl;
std::cout << "There are " << entries->size() << " entries in the map" << std::endl;

for (size_t i = 0; i < entries->size(); ++i) {
   const std::string * key = entries->getKey(i);
      if ((std::string *) NULL == key) {
			std::cout << "The key at index " << i << " is NULL" << std::endl;
        } else {
            std::auto_ptr<std::string> val = entries->releaseValue(i);
            std::cout << "(Key, Value) for index " << i << " is: (" << *key << ", " <<
                (val.get() == NULL ? "NULL" : *val) << ")" << std::endl;
        }
    }
    std::cout << "Finished" << std::endl;
```

Raw pointer API uses the `DataArray` and `EntryArray` interfaces which allow late deserialization of objects. The entry in the returned array is deserialized only when it is accessed. Please see the example code below:

```
// No deserialization here
std::auto_ptr<hazelcast::client::DataArray<std::string> > vals = map.values(); 

// deserializes the item at index 0 assuming that there are at least 1 items in the array
const std::string *value = vals->get(0);

// no deserialization here since it was already de-serialized
value = vals->get(0);

// no deserialization here since it was already de-serialized
value = (*vals)[0];

// releases the value so that you can keep this object pointer in your application at some other place
std::auto_ptr<std::string> releasedValue = vals->release(0);

// deserialization occurs again since the value was released already
value = vals->get(0);
```

Using the raw pointer based API may improve the performance if you are using the API to return multiple values such as `values`, `keySet` and `entrySet`. In this case, the cost of deserialization is delayed until the item is actually accessed.

## 7.11. Mixed Object Types Supporting HazelcastClient

Sometimes, you may need to use Hazelcast data structures with objects of different types. For example, you may want to put `int`, `string`, `IdentifiedDataSerializable`, etc. objects into the same Hazelcast `IMap` data structure. You can do this by using the mixed type adopted `HazelcastClient`. You can adopt the client in this way:
```
    ClientConfig config;
    HazelcastClient client(config);
    mixedtype::HazelcastClient &hazelcastClient = client.toMixedType();
``` 
The `mixedtype::HazelcastClient` interface is designed to provide you the data structures which allows you to work with any object types in a mixed manner. For example, the interface allows you to provide the key and value type differently for each `map.put` call. An example usage is shown below:
```
    mixedtype::IMap map = hazelcastClient.getMap("MyMap");

    map.put<int, int>(3, 5);
    map.put<string, std::string>("string key1", "MyStringValue");
    map.put<int, MyCustomObject>("string key1", myCustomInstance);
    TypedData result = map.get<int>(3);
``` 

As you can see in the above code snippet, we are putting `int`, `string` and `MyCustomObject` to the same map. Both the key and value can be of different type for each `map.put` call.

If you want to use a mixed type map with near cache, then you should use the `MixedNearCacheConfig` class and add this configuration to the `ClientConfig` using the `addMixedNearCacheConfig` method. See the below example:

```
   boost::shared_ptr<mixedtype::config::MixedNearCacheConfig> nearCacheConfig(new mixedtype::config::MixedNearCacheConfig("MixedMapTestMap"));
   clientConfig.addMixedNearCacheConfig(nearCacheConfig);
```
Mixed type support for near cache only exists when the in-memory format is BINARY. The OBJECT in-memory format is not supported for `MixedNearCacheConfig`.

The mixed type API uses the TypedData class at the user interface.

### 7.11.1. TypedData API

The TypedData class is a wrapper class for the serialized binary data. It presents the following user APIs:
```
            /**
             *
             * @return The type of the underlying object for this binary.
             */
            const serialization::pimpl::ObjectType getType() const;

            /**
             * Deserializes the underlying binary data and produces the object of type T.
             *
             * <b>CAUTION</b>: The type that you provide should be compatible with what object type is returned with
             * the getType API, otherwise you will either get an exception of incorrectly try deserialize the binary data.
             *
             * @tparam T The type to be used for deserialization
             * @return The object instance of type T.
             */
            template <typename T>
            std::auto_ptr<T> get() const;
```

TypedData does late deserialization of the data only when the get method is called.

This TypedData allows you to retrieve the data type of the underlying binary to be used when being deserialized. This class represents the type of a Hazelcast serializable object. The fields can take the following values:
1. <b>Primitive types</b>: `factoryId=-1`, `classId=-1`, `typeId` is the type ID for that primitive as listed in
`SerializationConstants`
2. <b>Array of primitives</b>: `factoryId=-1`, `classId=-1`, `typeId` is the type ID for that array as listed in
`SerializationConstants`
3. <b>IdentifiedDataSerializable</b>: `factoryId`, `classId` and `typeId` are non-negative values as registered by
the `DataSerializableFactory`.
4. <b>Portable</b>: `factoryId`, `classId` and `typeId` are non-negative values as registered by the `PortableFactory`.
5. <b>Custom serialized objects</b>: `factoryId=-1`, `classId=-1`, `typeId` is the non-negative type ID as 
registered for the custom object. 


# 8. Development and Testing

Hazelcast C++ client is developed using C++. If you want to help with bug fixes, develop new features or
tweak the implementation to your application's needs, you can follow the steps in this section.

## 8.1. Building and Using Client From Sources

For compiling with SSL support: 

- You need to have the OpenSSL (version 1.0.2) installed in your development environment: (i) add OpenSSL `include` directory to include directories, (ii) add OpenSSL `library` directory to the link directories list (this is the directory named `tls`, e.g., `cpp/Linux_64/hazelcast/lib/tls`), and (iii) set the OpenSSL libraries to link.
 
- You can provide your OpenSSL installation directory to cmake using the following flags:
    - -DHZ_OPENSSL_INCLUDE_DIR="Path to OpenSSL installation include directory" 
    - -DHZ_OPENSSL_LIB_DIR="Path to OpenSSL lib directory"

- Use -DHZ_COMPILE_WITH_SSL=ON

- Check the top level CMakeLists.txt file to see which libraries we link for OpenSSL in which environment. 
    - For Mac OS and Linux, we link with "ssl" and "crypto".
    - For Windows, install also the 1.1.x version of the OpenSSL library. An example linkage for 64-bit library and release build: "libeay32MD ssleay32MD libcrypto64MD libSSL64MD".


Follow the below steps to build and install Hazelcast C++ client from its source:

1. Clone the project from GitHub using the following command: `git clone --recursive git@github.com:hazelcast/hazelcast-cpp-client.git`.  We use the `--recursive` flag for our dependency on the googletest framework.
2. Create a build directory, e.g., `build`, from the root of the project. In the build directory, run the cmake command depending on your environment as depicted in the next sections.

### 8.1.1 Mac

**Release:**

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release

**Code Coverage:**

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug -DHZ_CODE_COVERAGE=ON
	gcovr -u -e .*external.* -e SimpleMapTest.h --html --html-details -o coverage.html 

**Xcode Development:**

  	cmake .. -G Xcode -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug

	cmake .. -G Xcode -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=archive -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=library

**Valgrind sample run with suppressions:**

        valgrind --leak-check=yes  --gen-suppressions=all --suppressions=../test.sup  ./hazelcast/test/clientTest_STATIC_64.exe  > log.t

### 8.1.2 Linux

**Release:**

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release

**Building the tests:**

    Add the -DHZ_BUILD_TESTS=ON flag to the cmake flags. e.g.:
    cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug -DHZ_BUILD_TESTS=ON

**Building the examples:**

    Add the -DHZ_BUILD_EXAMPLES=ON flag to the cmake flags. e.g.:
    cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug -DHZ_BUILD_EXAMPLES=ON

**Valgrind:**

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug -DHZ_VALGRIND=ON

### 8.1.3 Windows

**Release:**

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release
	cmake .. -G "Visual Studio 12 Win64"  -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release
	cmake .. -G "Visual Studio 12 Win64"  -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release

	MSBuild.exe HazelcastClient.sln /property:Configuration=Release

**Debug:**

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Debug
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Debug
	cmake .. -G "Visual Studio 12 Win64"  -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug
	cmake .. -G "Visual Studio 12 Win64"  -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug

	MSBuild.exe HazelcastClient.sln /property:TreatWarningsAsErrors=true /property:Configuration=Debug

## 8.2. Testing

In order to test Hazelcast C++ client locally, you will need the following:

* Java 6 or newer
* Maven
* cmake
* OpenSSL
* Python 2 and pip (Needed for RemoteController used for controlling server instances)

You can also pull our test docker images from the docker hub using the following command: `docker pull ihsan/gcc_346_ssl`. These images have all the tools for building the project.

Following command builds and runs the tests:

* Linux: `./testLinuxSingleCase.sh 64 SHARED Debug`
* MacOS: `./testLinuxSingleCase.sh 64 SHARED Debug`
* Windows: `./testWindowsSingleCase.bat 64 SHARED Debug`

### 8.2.1 Tested Platforms

Our CI tests run continuously on the following platforms and compilers:

* Linux: CentOs5 gcc 3.4.6, CentOs5.11 gcc 4.1.2, centos 7 gcc 4.8.2
* Windows: Visual Studio 12
* Mac OS: Apple LLVM version 7.3.0 (clang-703.0.31)

## 8.3 Reproducing Released Libraries

Sometimes you may want to reproduce the released library for your own compiler environment. You need to run the release script and it will produce a release folder named "cpp".

Note: The default release scripts require that you have OpenSSL (version 1.0.2) installed in your development environment.

## Mac

Run releaseOSX.sh.

## Linux

Run releaseLinux.sh

## Windows

Run releaseWindows.bat.


# 9. Getting Help

You can use the following channels for your questions and development/usage issues:

* This repository by opening an issue.
* Hazelcast Hazelcast channel on Gitter: 
[![Join the chat at https://gitter.im/hazelcast/hazelcast](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/hazelcast/hazelcast?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
* Our Google Groups directory: https://groups.google.com/forum/#!forum/hazelcast
* Stack Overflow: https://stackoverflow.com/questions/tagged/hazelcast

# 10. Contributing

Besides your development contributions as explained in the [Development and Testing chapter](#8-development-and-testing) above, you can always open a pull request on this repository for your other requests such as documentation changes.

# 11. License

[Apache 2 License](https://github.com/hazelcast/hazelcast-cpp-client/blob/master/LICENSE).

# 12. Copyright

Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.

Visit [www.hazelcast.com](http://www.hazelcast.com) for more information.
