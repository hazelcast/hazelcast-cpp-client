# Table of Contents

* [Generating API Documentation](#generating-api-documentation)
* [Downloading the Project](#downloading-the-project)
* [Building the Project](#building-the-project)
  * [Mac](#mac)
  * [Linux](#linux)
  * [Windows](#windows)
* [Features](#features)
* [Setting Up the Client](#setting-up-the-client)
* [Installing the Client](#installing-the-client)
* [Compiling the Client](#compiling-the-client)
  * [Mac Client](#mac-client)
  * [Linux Client](#linux-client)
  * [Windows  Client](#windows-client)
* [Serialization Support](#serialization-support)
  * [Custom Serialization](#custom-serialization) 
* [Raw Pointer API](#raw-pointer-api)
* [Query API](#query-api)
* [Ringbuffer](#ringbuffer)
* [Reliable Topic](#reliable-topic)
* [Code Examples](#code-examples)
  * [Map](#map)
  * [Queue](#queue)
  * [Entry Listener](#entry-listener)
  * [Serialization](#serialization)
  * [Continuous Query](#continuous-query)
  * [Ringbuffer](#ringbuffer)
  * [Reliable Topic](#reliable-topic)
    * [Publisher](#publisher)
    * [Subscriber](#subscriber)
* [Mail Group](#mail-group)
* [License](#license)
* [Copyright](#copyright)

This is the repository of C++ client implementation for [Hazelcast](https://github.com/hazelcast/hazelcast), the open source in-memory data grid. A comparison of features supported by the C++ Client vs the Java client can be found [here](http://docs.hazelcast.org/docs/latest-dev/manual/html-single/index.html#hazelcast-clients-feature-comparison).



# Generating API Documentation

You can generate API Documentation via Doxygen from root with the following command.

```
doxygen docsConfig
```

# Downloading the Project

Clone the project from github:

git clone --recursive git@github.com:hazelcast/hazelcast-cpp-client.git

We use --recursive flag for our dependency on googletest framework.

# Building the Project

First create a build directory from the root of the project. In the build directory, run the following commands depending on your environment.

## Mac

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

## Linux

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

## Windows

**Release:**

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release
	cmake .. -G "Visual Studio 10 Win64"  -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release
	cmake .. -G "Visual Studio 10 Win64"  -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release

	MSBuild.exe HazelcastClient.sln /property:Configuration=Release

**Debug:**

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Debug
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Debug
	cmake .. -G "Visual Studio 10 Win64"  -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug
	cmake .. -G "Visual Studio 10 Win64"  -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug

	MSBuild.exe HazelcastClient.sln /property:TreatWarningsAsErrors=true /property:Configuration=Debug

# Features




You can use Native C++ Client to connect to Hazelcast cluster members and perform almost all operations that a member can perform. Clients differ from members in that clients do not hold data. The C++ Client is by default a smart client, i.e., it knows where the data is and asks directly for the correct member. You can disable this feature (using the `ClientConfig::setSmart` method) if you do not want the clients to connect to every member.

The features of C++ Clients are listed below:

- Access to distributed data structures (IMap, IQueue, MultiMap, ITopic, etc.).
- Access to transactional distributed data structures (TransactionalMap, TransactionalQueue, etc.).
- Ability to add cluster listeners to a cluster and entry/item listeners to distributed data structures.
- Distributed synchronization mechanisms with ILock, ISemaphore and ICountDownLatch.


# Setting Up the Client

Hazelcast C++ Client is shipped with 32/64 bit, shared and static libraries. You only need to include the boost *shared_ptr.hpp* header in your compilation since the API makes use of the boost `shared_ptr`.


The downloaded release folder consists of:

- Mac_64/
- Windows_32/
- Windows_64/
- Linux_32/
- Linux_64/
- docs/ *(HTML Doxygen documents are here)*


Each of the folders above contains the following:

- examples/
	There are a number of examples in this folder for each feature. Each example produces an executable which you can run in a cluster. You may need to set the server IP addresses for the examples to run.

- hazelcast/
	- lib/ => Contains both shared and static library of hazelcast.
	- include/ => Contains headers of client.

- external/
	- include/ => Contains headers of dependencies. (boost::shared_ptr)

# Installing the Client

The C++ Client is tested on Linux 32/64-bit, Mac 64-bit and Windows 32/64-bit machines. For each of the headers above, it is assumed that you are in the correct folder for your platform. Folders are Mac_64, Windows_32, Windows_64, Linux_32 or Linux_64.

# Compiling the Client

For compilation, you need to include the `hazelcast/include` and `external/include` folders in your distribution. You also need to link your application to the appropriate static or shared library.

## Mac Client

For Mac, there is one distribution: 64 bit.

Here is an example script to build with static library:

`g++ main.cpp -I./external/include -I./hazelcast/include ./hazelcast/lib/libHazelcastClientStatic_64.a`

Here is an example script to build with shared library:

`g++ main.cpp -I./external/include -I./hazelcast/include -L./hazelcast/lib -lHazelcastClientShared_64`

## Linux Client

For Linux, there are two distributions: 32 bit and 64 bit.

Here is an example script to build with static library:

`g++ main.cpp -pthread -I./external/include -I./hazelcast/include
      ./hazelcast/lib/libHazelcastClientStatic_64.a`

Here is an example script to build with shared library:

`g++ main.cpp -lpthread -Wl,–no-as-needed -lrt -I./external/include -I./hazelcast/include -L./hazelcast/lib -lHazelcastClientShared_64`


## Windows Client

For Windows, there are two distributions; 32 bit and 64 bit. The static library is located in a folder named "static" while the dynamic library(dll) is in the folder named as "shared".

When compiling for Windows environment the user should specify one of the following flags:
    HAZELCAST_USE_STATIC: You want the application to use the static Hazelcast library.
    HAZELCAST_USE_SHARED: You want the application to use the shared Hazelcast library.

# Serialization Support

C++ client supports the following types of object serializations:

- **Built-in primitive types**: Some primitive types have built-in support for serialization. These are `char`, `unsigned char` (`byte`), `bool`, `short`, `int`, `long`, `float`, `double`, `stl string`and vector of these primitive types. 
- **IdentifiedDataSerializable**: This interface enables a fast serialization by providing a unique factory and class IDs. It requires the server side class as well.
- **Portable Serialization**: This serialization carries the meta data for the object structure. If server side deserialization is not needed, you do not need to prepare the server side implementation.
- **Custom Serialization**: This serialization allows you to use an external custom serialization, e.g., Google's Protocol Buffers. It provides serialization support without modifying your existing libraries where object classes exist. 

## Custom Serialization

If all of your classes that need to be serialized are inherited from the same class, you can use an implementation as shown in the example snippet below:

```
class  MyCustomSerializer : public serialization::Serializer<ExampleBaseClass> {
   public:
      void write(serialization::ObjectDataOutput & out, const ExampleBaseClass& object);
      void read(serialization::ObjectDataInput & in, ExampleBaseClass& object);
      int getHazelcastTypeId() const;
    };
```    

If your classes are not inherited from the same base class, you can use a serializer class with templates as shown in the example snippet below:

```
template<typename T>
class MyCustomSerializer : public serialization::Serializer<T> {
       public:
         void write(serialization::ObjectDataOutput & out, const T& object) {
                            //.....
         }
         void read(serialization::ObjectDataInput & in, T& object) {
                           //.....
         }
         int getHazelcastTypeId() const {
                           //..
         }
    };
```

Along with your serializer, you should provide the function `getHazelcastTypeId()` with the same namespace to which `ExampleBaseClass` belongs as shown below:

```
int getHazelcastTypeId(const MyClass*);
```

This function should return the same ID with its serializer. This ID is used to determine which serializer needs to be used for your classes. 

After you implement your serializer, you can register it using `SerializationConfig` as shown below:

```
clientConfig.getSerializationConfig().
registerSerializer(boost::shared_ptr<hazelcast::client::
serialization::SerializerBase>(new MyCustomSerializer());
```

# Raw Pointer API

When using C++ client you can have the ownership of raw pointers for the objects you create and return. This allows you to keep the objects in your library/application without any need for copy.

For each container you can use the adapter classes, whose names start with `RawPointer`, to access the raw pointers of the created objects. These adapter classes are found in `hazelcast::client::adaptor` namespace and listed below:

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

Raw pointer API uses the DataArray and EntryArray interfaces which allow late deserialization of objects. The entry in the returned array is deserialized only when it is accessed. Please see the example code below:

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

Using raw pointer based API may improve performance if you are using the API to return multiple values such as values, keySet, and entrySet. In this case, cost of deserialization is delayed until the item is actually accessed.

# Query API

C++ client API allows you to query map values, keys and entries using predicates. It also allows you to use Hazelcast Map's `executeOnKey` and `executeOnEntries` methods with predicates. You can run a processor on a subset of entries with these methods. 

You can add entry listeners with predicates using C++ client API. By this way, only the events for the selected subset of entries matching the query criteria are received by your listener.

C++ client API provides a rich set of built-in predicates as supported by the Java client. You can create your own predicates by implementing `Predicate` interfaces both at the C++ client side and server side. Built-in predicates are listed below:

- `AndPredicate`
- `EqualPredicate` 
- `ILikePredicate`
- `LikePredicate`
- `OrPredicate`
- `TruePredicate`
- `BetweenPredicate` 
- `FalsePredicate`
- `InPredicate`
- `NotEqualPredicate`
- `PagingPredicate`
- `RegexPredicate`
- `GreaterLessPredicate`
- `InstanceOfPredicate`
- `NotPredicate`
- `SqlPredicate`

An example query is shown in the following snippet:

```
IMap<int, int> intMap = client.getMap<int, int>("testValuesWithPredicateIntMap");
adaptor::RawPointerMap<int, int> rawMap(intMap);
// ...
// BetweenPredicate
// 5 <= key <= 10
valuesArray = rawMap.values(query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
```

This example query returns the values between 5 and 10, inclusive. You can find the examples of each built-in predicate in `distributed-map/query` folder of `examples`.


*NOTE: API that returns pointers may return null pointers for null values. You need to check for null values.*

# Ringbuffer

You can benefit from Hazelcast Ringbuffer using the C++ client library. You can start by obtaining the Ringbuffer using the `HazelcastClient` as usual, as shown below:

```
boost::shared_ptr<hazelcast::client::Ringbuffer<std::string> > rb = client.getRingbuffer<std::string>("myringbuffer");
```

Ringbuffer interface allows you to add a new item to the Ringbuffer or read an entry at a sequence number.

You can query the Ringbuffer capacity which is configured at the server side.

# Reliable Topic

You can use Reliable Topic if you do not want to miss any messages during failures. Hazelcast Reliable Topic provides a very similar interface to Topic structure but it has several configuration options.

Reliable Topic implementation depends on the Ringbuffer data structure. The data is kept in the Hazelcast cluster's Ringbuffer structure. These Ringbuffer structures' names start with "\_hz\_rb\_".
 
Reliable Topic also supports batch reads from the Ringbuffer. You can optimize the internal working of listener using the method `ReliableTopicConfig::setReadBatchSize`.

# Code Examples

You can try the following C++ client code examples. You need to have a Hazelcast client member running for the code examples to work. 

## Map

```cpp
#include <hazelcast/client/HazelcastAll.h>
#include <iostream>

using namespace hazelcast::client;

int main() {
  ClientConfig clientConfig;
  Address address( "localhost", 5701 );
  clientConfig.addAddress( address );

  HazelcastClient hazelcastClient( clientConfig );

  IMap<int,int> myMap = hazelcastClient.getMap<int ,int>( "myIntMap" );
  myMap.put( 1,3 );
  boost::shared_ptr<int> value = myMap.get( 1 );
  if( value.get() != NULL ) {
    //process the item
  }

  return 0;
}
```

## Queue

```cpp
#include <hazelcast/client/HazelcastAll.h>
#include <iostream>
#include <string>

using namespace hazelcast::client;

int main() {
  ClientConfig clientConfig;
  Address address( "localhost", 5701 );
  clientConfig.addAddress( address );

  HazelcastClient hazelcastClient( clientConfig );

  IQueue<std::string> queue = hazelcastClient.getQueue<std::string>( "q" );
  queue.offer( "sample" );
  boost::shared_ptr<std::string> value = queue.poll();
  if( value.get() != NULL ) {
    //process the item
  }
  return 0;
}
```

## Entry Listener

```cpp
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/HazelcastClient.h"
#include <iostream>
#include <string>

using namespace hazelcast::client;

class SampleEntryListener {
  public:

  void entryAdded( EntryEvent<std::string, std::string> &event ) {
    std::cout << "entry added " <<  event.getKey() << " "
        << event.getValue() << std::endl;
  };

  void entryRemoved( EntryEvent<std::string, std::string> &event ) {
    std::cout << "entry added " <<  event.getKey() << " " 
        << event.getValue() << std::endl;
  }

  void entryUpdated( EntryEvent<std::string, std::string> &event ) {
    std::cout << "entry added " <<  event.getKey() << " " 
        << event.getValue() << std::endl;
  }

  void entryEvicted( EntryEvent<std::string, std::string> &event ) {
    std::cout << "entry added " <<  event.getKey() << " " 
        << event.getValue() << std::endl;
  }
};


int main( int argc, char **argv ) {
  ClientConfig clientConfig;
  Address address( "localhost", 5701 );
  clientConfig.addAddress( address );

  HazelcastClient hazelcastClient( clientConfig );

  IMap<std::string,std::string> myMap = hazelcastClient
      .getMap<std::string ,std::string>( "myIntMap" );
  SampleEntryListener *  listener = new SampleEntryListener();

  std::string id = myMap.addEntryListener( *listener, true );
  // Prints entryAdded
  myMap.put( "key1", "value1" );
  // Prints updated
  myMap.put( "key1", "value2" );
  // Prints entryRemoved
  myMap.remove( "key1" );
  // Prints entryEvicted after 1 second
  myMap.put( "key2", "value2", 1000 );

  // WARNING: deleting listener before removing it from Hazelcast leads to crashes.
  myMap.removeEntryListener( id );
  
  // listen using predicates
  // only listen the events for entries which has the value that matches the 
  // string "%VALue%1%", i.e. any string containing the text value1 case insensitive
  id = myMap.addEntryListener(*listener, query::ILikePredicate(
        query::QueryConstants::getValueAttributeName(), "%VALue%1%"), true);
  
  // this will generate an event
  myMap.put("key1", "my__value1_new" );
  
  sleep(1);
    
  myMap.removeEntryListener( id );
    
  // Delete listener after removing it from Hazelcast.
  delete listener;
  return 0;
};
```

## Serialization

Assume that you have the following two classes in Java and you want to use them with a C++ client. 

```java
class Foo implements Serializable {
  private int age;
  private String name;
}

class Bar implements Serializable {
  private float x;
  private float y;
} 
```

**First**, let them implement `Portable` or `IdentifiedDataSerializable` as shown below.

```java
class Foo implements Portable {
  private int age;
  private String name;

  public int getFactoryId() {
    // a positive id that you choose
    return 123;
  }

  public int getClassId() {
    // a positive id that you choose
    return 2;     
  }

  public void writePortable( PortableWriter writer ) throws IOException {
    writer.writeUTF( "n", name );
    writer.writeInt( "a", age );
  }

  public void readPortable( PortableReader reader ) throws IOException {
    name = reader.readUTF( "n" );
    age = reader.readInt( "a" );
  }
}

class Bar implements IdentifiedDataSerializable {
  private float x;
  private float y;

  public int getFactoryId() {
    // a positive id that you choose
    return 4;     
  }

  public int getId() {
    // a positive id that you choose
    return 5;    
  }

  public void writeData( ObjectDataOutput out ) throws IOException {
    out.writeFloat( x );
    out.writeFloat( y );
  }

  public void readData( ObjectDataInput in ) throws IOException {
    x = in.readFloat();
    y = in.readFloat();
  }
}
```

**Then**, implement the corresponding classes in C++ with same factory and class ID as shown below.

```cpp
class Foo : public Portable {
  public:
  int getFactoryId() const {
    return 123;
  };

  int getClassId() const {
    return 2;
  };

  void writePortable( serialization::PortableWriter &writer ) const {
    writer.writeUTF( "n", name );
    writer.writeInt( "a", age );
  };

  void readPortable( serialization::PortableReader &reader ) {
    name = reader.readUTF( "n" );
    age = reader.readInt( "a" );
  };

  private:
  int age;
  std::string name;
};

class Bar : public IdentifiedDataSerializable {
  public:
  int getFactoryId() const {
    return 4;
  };

  int getClassId() const {
    return 2;
  };

  void writeData( serialization::ObjectDataOutput& out ) const {
    out.writeFloat(x);
    out.writeFloat(y);
  };

  void readData( serialization::ObjectDataInput& in ) {
    x = in.readFloat();
    y = in.readFloat();
  };
  
  private:
  float x;
  float y;
};
```

Now, you can use the classes `Foo` and `Bar` in distributed structures. For example, you can use as Key or Value of `IMap` or as an Item in `IQueue`.

## Continuous Query

You can register an `EntryListener` for a map that will be triggered only when the specific map entries you choose are affected. This allows you to do more optimal queries.

Let’s start with an example.

```
1.  intMap = client->getMap&lt;int, int>("IntMap");
2.
3.  MyListener listener;
4.
5.  // 5 &lt;=key &lt;= 8
6.  std::string listenerId = intMap.addEntryListener(listener, query::BetweenPredicate&lt;int>(
7.         query::QueryConstants::getKeyAttributeName(), 5, 8), true);
8.
9.  intMap.put(1, 1);
10. intMap.put(8, 17);
11. intMap.put(5, 12, 1000); // evict after 1 second
12. intMap.remove(1);
13.
14. util::sleep(2);
15.
16. intMap.get(5); // trigger eviction
```

In this example, we register an `EntryListener` that only retrieves map events for entries with keys between 5 and 8 (inclusive). If an entry within this key range is added/updated/removed/evicted, you will be notified through the registered listener.

**Line 6:** Creates a built-in `BetweenPredicate` (see [Query API](http://docs.hazelcast.org/docs/latest-dev/manual/html-single/index.html#query-api) and [Query Example](https://github.com/hazelcast/hazelcast-cpp-client/blob/master/examples/distributed-map/query/queryExample.cpp) for all built-in predicates) and an `EntryListener` is added for listening to entry events with keys 5 through 8. The method “`query::QueryConstants::getKeyAttributeName()`” is used to tell that the query is being performed on the key of the entry. Similarly, you can use the method “`query::QueryConstants::getValueAttributeName()`” if you want to query on the value of the entry.

**Line 9:** Puts an entry for key = 1. This does not match your listener predicate, hence no event is received for this line.

**Line 10:** Puts an entry for key = 8. This key matches our interested key range, so you receive an event for the entry addition, and the `entryAdded` method of our listener will be called.

**Line 11:** Adds an entry for key = 5 with an expiry timeout of 1,000 milliseconds. This operation triggers an `entryAdded` event for our listener since the key is in our interested range.

**Line 12:** Removal of entry with key = 1 does not trigger any event and thus you will not receive any callback to your listener.

**Line 14:** Sets sleep for enough time that the entry with key = 5 will be evicted.

**Line 16:** Triggers an eviction for entry with key = 5. This line will cause an `entryEvicted` event to be received by your listener.

As shown in the above example, by using an `EntryListener` with predicates you can minimize the events to only ones in which you are interested. This is also known as the “Continuous Query” feature. You will keep receiving all events that match your query (which is the predicate that you provide on registration of your listener) while entries are being modified in the map in real time. The filtering is performed at the server side, so this method is a lot faster than receiving all the events and filtering at the client side.

Please examine all different kinds of built in predicates that we provide in the code examples. You can just grab a predicate or combine them using `AndPredicate` or `OrPredicate`. This provides you the capability to write complex queries. You cannot only query on keys or values, as explained above, but you can also use properties of an object for querying. The object can be the key as well as the value. Please refer to the [Query API section](http://docs.hazelcast.org/docs/latest-dev/manual/html-single/index.html#query-api) of the Hazelcast Reference Manual for details on queries.

Please be careful with the runtime behavior of your listener implementations to ensure that the listener callback methods return very quickly. If you need to perform long running tasks when an entry is changed, please off-load those operations to another thread.

In addition to this rich set of built-in predicates, you can also write your own custom queries simply by implementing your own predicate (you also have to implement the Java server-side predicate as well to make this work).

## Ringbuffer

```
    boost::shared_ptr<hazelcast::client::Ringbuffer<std::string> > rb = client.getRingbuffer<std::string>("myringbuffer");

    std::cout << "Capacity of the ringbuffer is:" << rb->capacity() << std::endl;

    int64_t sequenceNumber = rb->add("First Item");

    std::cout << "Added the first item at sequence " << sequenceNumber << std::endl;

    rb->add("Second item");

    std::cout << "There are " << rb->size() << " items in the ring buffer " << std::endl;

    std::auto_ptr<std::string> val = rb->readOne(sequenceNumber);

    if ((std::string *)NULL != val.get()) {
        std::cout << "The item at read at sequence " << sequenceNumber << " is " << *val << std::endl;
    }

    std::cout << "Finished" << std::endl;
```

## Reliable Topic

### Publisher

```
void publishWithDefaultConfig() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient client(config);

    boost::shared_ptr<hazelcast::client::ReliableTopic<std::string> > topic = client.getReliableTopic<std::string>("MyReliableTopic");
    std::string message("My first message");
    topic->publish(&message);
}

void publishWithNonDefaultConfig() {
    hazelcast::client::ClientConfig clientConfig;
    std::string topicName("MyReliableTopic");
    hazelcast::client::config::ReliableTopicConfig reliableTopicConfig(topicName.c_str());
    reliableTopicConfig.setReadBatchSize(5);
    clientConfig.addReliableTopicConfig(reliableTopicConfig);
    hazelcast::client::HazelcastClient client(clientConfig);

    boost::shared_ptr<hazelcast::client::ReliableTopic<std::string> > topic = client.getReliableTopic<std::string>(topicName);

    std::string message("My first message");

    topic->publish(&message);
}
```
### Subscriber

To write a subscriber, you need to implement the interface `hazelcast::client::topic::ReliableMessageListener`.

```
class MyListener : public hazelcast::client::topic::ReliableMessageListener<std::string> {
public:
    MyListener() : startSequence(-1), numberOfMessagesReceived(0), lastReceivedSequence(-1) {
    }

    MyListener(int64_t sequence) : startSequence(sequence), numberOfMessagesReceived(0), lastReceivedSequence(-1) {
    }

    virtual ~MyListener() {
    }

    virtual void onMessage(std::auto_ptr<hazelcast::client::topic::Message<std::string> > message) {
        ++numberOfMessagesReceived;

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

    int getNumberOfMessagesReceived() {
        int value = numberOfMessagesReceived;
        return value;
    }

private:
    int64_t startSequence;
    hazelcast::util::AtomicInt numberOfMessagesReceived;
    int64_t lastReceivedSequence;
};    
```

Using this listener, you can subscribe for the topic as shown below:

```
void listenWithDefaultConfig() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient client(config);

    std::string topicName("MyReliableTopic");
    boost::shared_ptr<hazelcast::client::ReliableTopic<std::string> > topic = client.getReliableTopic<std::string>(topicName);

    MyListener listener;
    const std::string &listenerId = topic->addMessageListener(listener);

    std::cout << "Registered the listener with listener id:" << listenerId << std::endl;

    while (listener.getNumberOfMessagesReceived() < 1) {
        hazelcast::util::sleep(1);
    }

    if (topic->removeMessageListener(listenerId)) {
        std::cout << "Successfully removed the listener " << listenerId << " for topic " << topicName << std::endl;
    } else {
        std::cerr << "Failed to remove the listener " << listenerId << " for topic " << topicName << std::endl;
    }
}

void listenWithConfig() {
    hazelcast::client::ClientConfig clientConfig;
    std::string topicName("MyReliableTopic");
    hazelcast::client::config::ReliableTopicConfig reliableTopicConfig(topicName.c_str());
    reliableTopicConfig.setReadBatchSize(5);
    clientConfig.addReliableTopicConfig(reliableTopicConfig);
    hazelcast::client::HazelcastClient client(clientConfig);

    boost::shared_ptr<hazelcast::client::ReliableTopic<std::string> > topic = client.getReliableTopic<std::string>(topicName);

    MyListener listener;
    const std::string &listenerId = topic->addMessageListener(listener);

    std::cout << "Registered the listener with listener id:" << listenerId << std::endl;

    while (listener.getNumberOfMessagesReceived() < 1) {
        hazelcast::util::sleep(1);
    }
    if (topic->removeMessageListener(listenerId)) {
        std::cout << "Successfully removed the listener " << listenerId << " for topic " << topicName << std::endl;
    } else {
        std::cerr << "Failed to remove the listener " << listenerId << " for topic " << topicName << std::endl;
    }
}    
```




# Mail Group

Please join the mail group if you are interested in using or developing Hazelcast.

[http://groups.google.com/group/hazelcast](http://groups.google.com/group/hazelcast)

# License

Hazelcast C++ Client is available under the Apache 2 License. Please see the [Licensing appendix](http://docs.hazelcast.org/docs/latest/manual/html-single/hazelcast-documentation.html#license-questions) for more information.

# Copyright


Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.

Visit [www.hazelcast.com](http://www.hazelcast.com/) for more information.
