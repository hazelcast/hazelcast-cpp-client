/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef HAZELCAST_CLIENT
#define HAZELCAST_CLIENT

#include <memory>

#include "hazelcast/client/map/impl/MixedMapProxyFactory.h"
#include "hazelcast/client/map/impl/ClientMapProxyFactory.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"
#include "hazelcast/client/proxy/RingbufferImpl.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/client/MixedMap.h"
#include "hazelcast/client/MultiMap.h"
#include "hazelcast/client/MixedMultiMap.h"
#include "hazelcast/client/IQueue.h"
#include "hazelcast/client/MixedQueue.h"
#include "hazelcast/client/ISet.h"
#include "hazelcast/client/MixedSet.h"
#include "hazelcast/client/IList.h"
#include "hazelcast/client/MixedList.h"
#include "hazelcast/client/ITopic.h"
#include "hazelcast/client/TransactionOptions.h"
#include "hazelcast/client/TransactionContext.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/ProxyManager.h"
#include "hazelcast/client/Ringbuffer.h"
#include "hazelcast/client/ReliableTopic.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
/*! \mainpage Introduction
 *
 * You can use native C++ Client to connect to hazelcast nodes and make almost all operations that a node does.
 * Different from nodes, clients do not hold data.
 *
 * Some of features of C++ Clients are:
 * * Access to distributed data structures like IMap, IQueue, MultiMap, ITopic etc... For complete list see the classes
 * extending DistributedObject
 * * Access to transactional distributed data structures like TransactionalMap, TransactionalQueue etc...
 * * Ability to add cluster listeners to a cluster and entry/item listeners to distributed data structures.
 * @see MembershipListener, IMap#addEntryListener , IQueue#addItemListener etc .
 * * Distributed synchronization mechanisms with ILock , ISemaphore and ICountDownLatch.
 * * C++ Client is smart by default, which means that it knows where the data is and asks directly to correct node.
 * Note that you can turn this feature off ( ClientConfig#setSmart), if you don't want your clients to connect every
 * node.
 *
 * Our C++ client is completely open source and the source code is freely available at https://github.com/hazelcast/hazelcast-cpp-client .
 * Please feel free to contribute. You can join our community at https://groups.google.com/forum/#!forum/hazelcast where
 * you can find answers to your questions.
 *
 * \section how_to_setup How to Setup
 *
 * Hazelcast C++ client is shipped with 32 & 64 bit, shared and static libraries. Compiled static libraries of dependencies
 * are also available in the release.
 *
 *  The user should make sure that the libraries are in appropriate linkage path for the application.
 *
 * Only dependency is shared_ptr from the boost library.
 *
 *  Downloaded release folder consist of
 *  * Windows_32/
 *  * Windows_64/
 *  * Linux_32/
 *  * Linux_64/
 *  * Mac_64/
 *  * docs/            => html doxygen documentations are here.
 *
 *  And each of the folders above (except docs folder) contains the following
 *  * examples/        => Contains code samples for the C++ API usage.
 *
 *  *  hazelcast/
 *      +  lib/        => Contains both shared and static library of hazelcast.
 *      +  include/    => Contains headers of client
 *  * external/
 *      + include/     => Contains headers of dependencies.(zlib and boost::shared_ptr)
 *
 *
 * \section guides Platform Specific Installation Guides
 *
 * C++ client is tested on Linux 32/64, Mac 64 and Windows 32/64 bit machines.
 * For each of the headers above, it is assumed that you are in correct folder for your platform.
 * Binaries that are distributed with enterprise zip are for following platforms:
 * Windows_32 (Visual Studio 2012), Windows_64 (Visual Studio 2012), Linux_32(gcc 3.4+ , libc 2.5+)
 * or Linux_64(gcc 3.4+ , libc 2.5+)
 * For other platforms are specific compiler versions, please contact support@hazelcast.com
 *
 *
 * \subsection Linux
 *
 * For linux, there are two distributions one is 32bit the other is 64bit.
 *
 * Sample script to build with static library:
 *
 *  * ﻿g++ main.cpp -pthread -I./external/include -I./hazelcast/include ./hazelcast/lib/libHazelcastClientStatic_64.a
 *
 * Sample script to build with shared library:
 *
 *  * g++ main.cpp -lpthread -Wl,--no-as-needed -lrt -I./external/include -I./hazelcast/include -L./hazelcast/lib -lHazelcastClientShared_64
 *
 * \subsection Mac
 *
 * For Mac binaries, please contact support@hazelcast.com
 *
 * Sample script to build with static library:
 *
 *  * g++ main.cpp -I./external/include -I./hazelcast/include ./hazelcast/lib/libHazelcastClientStatic_64.a
 *
 * Sample script to build with shared library:
 *
 *  * g++ main.cpp -I./external/include -I./hazelcast/include -L./hazelcast/lib -lHazelcastClientShared_64
 *
 * \subsection Windows
 *
 * For Windows, there are two distributions one is 32bit the other is 64bit.
 * Currently, the release is built with Visual Studio 2012 . For other builds, please contact with support@hazelcast.com
 *
 * When compiling for Windows environment the user should specify one of the following flags:
 *  * HAZELCAST_USE_STATIC: You want the application to use the static Hazelcast library.
 *  * HAZELCAST_USE_SHARED: You want the application to use the shared Hazelcast library.
 *
 *
 * \section code_samples Code Samples
 *
 * Note that these codes to work, there should be a Hazelcast node is running.
 *
 * The binaries for the examples are located at examples folder. The sources for the binaries are located at examples/src folder.
 *
 * You can also browse all the examples at https://github.com/hazelcast/hazelcast-cpp-client/tree/master/examples.
 *
 * Furthermore, you can access a large set of tests that we use for testing the API, these will also give you a better idea about
 * how we are using the API. You can find these tests at https://github.com/hazelcast/hazelcast-cpp-client/tree/master/hazelcast/test/src
 *
 * The CommandLineTool example gives you a way to play with the data in the cluster from the command line terminal.
 *
 * \subsection map Map example
 *
 *          #include "(.*)"
 *          #include <iostream>
 *
 *          using namespace hazelcast::client;
 *
 *          int main(){
 *              ClientConfig clientConfig;
 *              Address address("localhost", 5701);
 *              clientConfig.addAddress(address);
 *
 *              HazelcastClient hazelcastClient(clientConfig);
 *
 *              IMap<int,int> myMap = hazelcastClient.getMap<int ,int>("myIntMap");
 *              myMap.put(1,3);
 *              boost::shared_ptr<int> v = myMap.get(1);
 *              if(v.get() != NULL){
 *                  //process the item
 *              }
 *
 *              // You can work with raw pointer maps if you want to get memory ownership of returned objects
 *              hazelcast::client::adaptor::RawPointerMap<int, int> rawMap(myMap);
 *              myMap.put(2, 6);
 *              std::auto_ptr<int> value = myMap.get(1);
 *              if (NULL != value.get()) {
 *                  // do something with the value
 *              }
 *
 *              // query values with predicate
 *              // EqualPredicate
 *              // key == 5
 *              std::vector<int> values = myMap.values(query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
 *              // Same query with raw Map
 *              std::auto_ptr<DataArray<int> > valuesArray = rawMap.values(query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
 *              size_t numberOfValues = valuesArray->size();
 *              if (numberOfValues > 0) {
 *                  const int *firstValue = valuesArray->get(0);
 *                  firstValue = (*valuesArray)[0];
 *                  std::auto_ptr<int> firstValueWithMemoryOwnership = valuesArray->release(0);
 *              }
 *
 *              // add listener with predicate
 *              // Only listen events for entries with key >= 7
 *              MyEntryListener listener;
 *              std::string listenerId = map.addEntryListener(listener,
 *                                   hazelcast::client::query::GreaterLessPredicate<int>(
 *                                   hazelcast::client::query::QueryConstants::getKeyAttributeName(), 7, true, false), true);
 *
 *              // same listener using the raw map
 *              std::string secondListener = rawMap.addEntryListener(listener,
 *                                   hazelcast::client::query::GreaterLessPredicate<int>(
 *                                   hazelcast::client::query::QueryConstants::getKeyAttributeName(), 7, true, false), true);
 *
 *              // listen for entries
 *              sleep(10);
 *
 *              return 0;
 *          }
 *
 * \subsection queue Queue Example
 *
 *          #include "(.*)"
 *          #include <iostream>
 *          #include <string>
 *
 *          using namespace hazelcast::client;
 *
 *          int main(){
 *              ClientConfig clientConfig;
 *              Address address("localhost", 5701);
 *              clientConfig.addAddress(address);
 *
 *              HazelcastClient hazelcastClient(clientConfig);
 *
 *              IQueue<std::string> q = hazelcastClient.getQueue<std::string>("q");
 *              q.offer("sample");
 *              boost::shared_ptr<std::string> v = q.poll();
 *              if(v.get() != NULL){
 *                  //process the item
 *              }
 *              return 0;
 *          }
 *
 * \subsection entry_listener Entry Listener Example
 *
 *      #include "(.*)"
 *      #include <iostream>
 *      #include <string>
 *
 *      using namespace hazelcast::client;
 *
 *      class SampleEntryListener {
 *      public:
 *
 *          void entryAdded(EntryEvent<std::string, std::string> &event) {
 *              std::cout << "entry added " <<  event.getKey() << " " << event.getValue() << std::endl;
 *          };
 *
 *          void entryRemoved(EntryEvent<std::string, std::string> &event) {
 *              std::cout << "entry added " <<  event.getKey() << " " << event.getValue() << std::endl;
 *          }
 *
 *          void entryUpdated(EntryEvent<std::string, std::string> &event) {
 *              std::cout << "entry added " <<  event.getKey() << " " << event.getValue() << std::endl;
 *          }
 *
 *          void entryEvicted(EntryEvent<std::string, std::string> &event) {
 *              std::cout << "entry added " <<  event.getKey() << " " << event.getValue() << std::endl;
 *          }
 *      };
 *
 *
 *      int main(int argc, char **argv) {
 *
 *          ClientConfig clientConfig;
 *          Address address("localhost", 5701);
 *          clientConfig.addAddress(address);
 *
 *          HazelcastClient hazelcastClient(clientConfig);
 *
 *          IMap<std::string,std::string> myMap = hazelcastClient.getMap<std::string ,std::string>("myIntMap");
 *          SampleEntryListener *  listener = new SampleEntryListener();
 *
 *          std::string id = myMap.addEntryListener(*listener, true);
 *          myMap.put("key1", "value1"); //prints entryAdded
 *          myMap.put("key1", "value2"); //prints updated
 *          myMap.remove("key1"); //prints entryRemoved
 *          myMap.put("key2", "value2",1000); //prints entryEvicted after 1 second
 *
 *          myMap.removeEntryListener(id); //WARNING: deleting listener before removing it from hazelcast leads to crashes.
 *          delete listener;               //delete listener after remove it from hazelcast.
 *          return 0;
 *      };
 *
 * \subsection serialization Serialization Example
 *
 * Suppose you have the following two classes in Java and you want to use it with C++ client.
 *
 *      class Foo implements Serializable{
 *          private int age;
 *          private String name;
 *      }
 *
 *      class Bar implements Serializable{
 *          private float x;
 *          private float y;
 *      }
 *
 * First make them implement Portable or IdentifiedDataSerializable.
 *
 *
        class Foo implements Portable {
            private int age;
            private String name;

            public int getFactoryId() {
                return 666;   // a positive id that you choose
            }

            public int getClassId() {
                return 2;     // a positive id that you choose
            }

            public void writePortable(PortableWriter writer) throws IOException {
                writer.writeUTF("n", &name);
                writer.writeInt("a", age);
            }

            public void readPortable(PortableReader reader) throws IOException {
                name = reader.readUTF("n");
                age = reader.readInt("a");
            }
        }

        class Bar implements IdentifiedDataSerializable {
            private float x;
            private float y;

            public int getFactoryId() {
                return 4;     // a positive id that you choose
            }

            public int getId() {
                return 5;    // a positive id that you choose
            }

            public void writeData(ObjectDataOutput out) throws IOException {
                out.writeFloat(x);
                out.writeFloat(y);
            }

            public void readData(ObjectDataInput in) throws IOException {
                x = in.readFloat();
                y = in.readFloat();
            }
        }

 *
 * Then, implement the corresponding classes in C++ with same factory and class Id as follows:
 *
        class Foo : public serialization::Portable {
        public:
            int getFactoryId() const{
                return 666;
            };

            int getClassId() const{
                return 2;
            };

            void writePortable(serialization::PortableWriter &writer) const{
                writer.writeUTF("n", &name);
                writer.writeInt("a", age);
            };

            void readPortable(serialization::PortableReader &reader){
                name = reader.readUTF("n");
                age = reader.readInt("a");
            };

        private:
            int age;
            std::string name;
        };

        class Bar : public serialization::IdentifiedDataSerializable {
            public:
                int getFactoryId() const{
                    return 4;
                };

                int getClassId() const{
                    return 2;
                };

                void writeData(serialization::ObjectDataOutput& out) const{
                    out.writeFloat(x);
                    out.writeFloat(y);
                };

                void readData(serialization::ObjectDataInput& in){
                    x = in.readFloat();
                    y = in.readFloat();
                };
            private:
                float x;
                float y;
         };
 *
 *  Now, you can use class Foo and Bar in distributed structures. For example as Key or Value of IMap,
 *  or as an Item in IQueue.
 *
 */
        namespace connection {
            class ConnectionManager;
        }

        namespace serialization {
            namespace pimpl {
                class SerializationService;
            }
        }
        namespace spi {
            class ClientContext;

            class InvocationService;

            class ClusterService;

            class PartitionService;

            class LifecycleService;

            class ServerListenerService;

            class ClientProxyFactory;

        }

        class ClientConfig;

        class IdGenerator;

        class IAtomicLong;

        class ICountDownLatch;

        class ISemaphore;

        class ILock;

        class TransactionContext;

        class TransactionOptions;

        class Cluster;

        /**
        * Hazelcast Client enables you to do all Hazelcast operations without
        * being a member of the cluster. It connects to one of the
        * cluster members and delegates all cluster wide operations to it.
        * When the connected cluster member dies, client will
        * automatically switch to another live member.
        */
        class HAZELCAST_API HazelcastClient {
            friend class spi::ClientContext;

        public:
            /**
            * Constructs a hazelcastClient with given ClientConfig.
            * Note: ClientConfig will be copied.
            * @param config client configuration to start the client with
            */
            HazelcastClient(ClientConfig &config);

            /**
            * Destructor
            */
            ~HazelcastClient();

            /**
            *
            * @tparam T type of the distributed object
            * @param name name of the distributed object.
            * @returns distributed object
            */
            template<typename T>
            T getDistributedObject(const std::string& name) {
                T t(name, &(clientContext));
                return t;
            }

            /**
            *
            * Returns the distributed map instance with the specified name.
            *
            * @tparam K key type
            * @tparam V value type
            * @param name name of the distributed map
            * @return distributed map instance with the specified name
            */
            template<typename K, typename V>
            IMap<K, V> getMap(const std::string &name) {
                map::impl::ClientMapProxyFactory<K, V> factory(&clientContext);
                boost::shared_ptr<spi::ClientProxy> proxy =
                        getDistributedObjectForService(IMap<K, V>::SERVICE_NAME, name, factory);

                return IMap<K, V>(proxy);
            }

            MixedMap getMixedMap(const std::string &name);

            /**
            * Returns the distributed multimap instance with the specified name.
            *
            * @param name name of the distributed multimap
            * @return distributed multimap instance with the specified name
            */
            template<typename K, typename V>
            MultiMap<K, V> getMultiMap(const std::string& name) {
                return getDistributedObject<MultiMap<K, V> >(name);
            }

            /**
            * Returns the distributed multimap instance with the specified name.
            *
            * @param name name of the distributed multimap
            * @return distributed multimap instance with the specified name
            */
            MixedMultiMap getMixedMultiMap(const std::string& name) {
                return getDistributedObject<MixedMultiMap>(name);
            }

            /**
            * Returns the distributed queue instance with the specified name and entry type E.
            *
            * @param name name of the distributed queue
            * @return distributed queue instance with the specified name
            */
            template<typename E>
            IQueue<E> getQueue(const std::string& name) {
                return getDistributedObject<IQueue<E> >(name);
            }

            /**
            * Returns the distributed queue instance with the specified name and entry type E.
            *
            * @param name name of the distributed queue
            * @return distributed queue instance with the specified name
            */
            MixedQueue getMixedQueue(const std::string& name) {
                return getDistributedObject<MixedQueue>(name);
            }

            /**
            * Returns the distributed set instance with the specified name and entry type E.
            * Set is ordered unique set of entries. similar to std::set
            *
            * @param name name of the distributed set
            * @return distributed set instance with the specified name
            */

            template<typename E>
            ISet<E> getSet(const std::string& name) {
                return getDistributedObject<ISet<E> >(name);
            }

            /**
            * Returns the distributed set instance with the specified name and entry type E.
            * Set is ordered unique set of entries. similar to std::set
            *
            * @param name name of the distributed set
            * @return distributed set instance with the specified name
            */
            MixedSet getMixedSet(const std::string& name) {
                return getDistributedObject<MixedSet>(name);
            }

            /**
            * Returns the distributed list instance with the specified name.
            * List is ordered set of entries. similar to std::vector
            *
            * @param name name of the distributed list
            * @return distributed list instance with the specified name
            */
            template<typename E>
            IList<E> getList(const std::string& name) {
                return getDistributedObject<IList<E> >(name);
            }

            /**
            * Returns the distributed list instance with the specified name.
            * List is ordered set of entries. similar to std::vector
            *
            * @param name name of the distributed list
            * @return distributed list instance with the specified name
            */
            MixedList getMixedList(const std::string& name) {
                return getDistributedObject<MixedList>(name);
            }

            /**
            * Returns the distributed topic instance with the specified name and entry type E.
            *
            * @param name name of the distributed topic
            * @return distributed topic instance with the specified name
            */
            template<typename E>
            ITopic<E> getTopic(const std::string& name) {
                return getDistributedObject<ITopic<E> >(name);
            };

            /**
            * Returns the distributed topic instance with the specified name and entry type E.
            *
            * @param name name of the distributed topic
            * @return distributed topic instance with the specified name
            */
            template<typename E>
            boost::shared_ptr<ReliableTopic<E> > getReliableTopic(const std::string& name) {
                boost::shared_ptr<Ringbuffer<topic::impl::reliable::ReliableTopicMessage> > rb =
                        getRingbuffer<topic::impl::reliable::ReliableTopicMessage>(TOPIC_RB_PREFIX + name);
                return boost::shared_ptr<ReliableTopic<E> >(new ReliableTopic<E>(name, &clientContext, rb));
            }

            /**
            * Creates cluster-wide unique IDs. Generated IDs are long type primitive values
            * between <tt>0</tt> and <tt>LONG_MAX</tt> . Generated IDs are unique during the life
            * cycle of the cluster. If the entire cluster is restarted, IDs start from <tt>0</tt> again.
            *
            * @param name name of the IdGenerator
            * @return IdGenerator for the given name
            */
            IdGenerator getIdGenerator(const std::string& name);

            /**
            * Creates cluster-wide atomic long. Hazelcast IAtomicLong is distributed
            * implementation of <tt>java.util.concurrent.atomic.AtomicLong</tt>.
            *
            * @param name name of the IAtomicLong proxy
            * @return IAtomicLong proxy for the given name
            */
            IAtomicLong getIAtomicLong(const std::string& name);

            /**
            * Creates cluster-wide CountDownLatch. Hazelcast ICountDownLatch is distributed
            * implementation of <tt>java.util.concurrent.CountDownLatch</tt>.
            *
            * @param name name of the ICountDownLatch proxy
            * @return ICountDownLatch proxy for the given name
            */

            ICountDownLatch getICountDownLatch(const std::string& name);

            /**
            * Returns the distributed lock instance for the specified key object.
            * The specified object is considered to be the key for this lock.
            * So keys are considered equals cluster-wide as long as
            * they are serialized to the same byte array such as String, long,
            * Integer.
            *
            * Locks are fail-safe. If a member holds a lock and some of the
            * members go down, cluster will keep your locks safe and available.
            * Moreover, when a member leaves the cluster, all the locks acquired
            * by this dead member will be removed so that these locks can be
            * available for live members immediately.
            *
            *      Lock lock = hazelcastInstance.getLock("PROCESS_LOCK");
            *      lock.lock();
            *      try {
            *        // process
            *      } finally {
            *        lock.unlock();
            *      }
            *
            * @param name name of the lock instance
            * @return distributed lock instance for the specified name.
            */
            ILock getILock(const std::string& name);

            /**
             * Returns the distributed Ringbuffer instance with the specified name.
             *
             * @param name name of the distributed Ringbuffer
             * @return distributed RingBuffer instance with the specified name
             */
            template <typename E>
            boost::shared_ptr<Ringbuffer<E> > getRingbuffer(const std::string& name) {
                return boost::shared_ptr<Ringbuffer<E> >(new proxy::RingbufferImpl<E>(name, &clientContext));
            }

            /**
            * Creates cluster-wide semaphore. Hazelcast ISemaphore is distributed
            * implementation of <tt>java.util.concurrent.Semaphore</tt>.
            *
            * @param name name of the ISemaphore proxy
            * @return ISemaphore proxy for the given name
            */
            ISemaphore getISemaphore(const std::string& name);

            /**
            *
            * @return configuration of this Hazelcast client.
            */
            ClientConfig& getClientConfig();

            /**
            * Creates a new TransactionContext associated with the current thread using default options.
            *
            * @return new TransactionContext
            */
            TransactionContext newTransactionContext();

            /**
            * Creates a new TransactionContext associated with the current thread with given options.
            *
            * @param options options for this transaction
            * @return new TransactionContext
            */
            TransactionContext newTransactionContext(const TransactionOptions& options);

            /**
            * Returns the Cluster that connected Hazelcast instance is a part of.
            * Cluster interface allows you to add listener for membership
            * events and learn more about the cluster.
            *
            * @return cluster
            */
            Cluster& getCluster();

            /**
            * Add listener to listen lifecycle events.
            *
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            * @param lifecycleListener Listener object
            */
            void addLifecycleListener(LifecycleListener *lifecycleListener);

            /**
            * Remove lifecycle listener
            * @param lifecycleListener
            * @return true if removed successfully
            */
            bool removeLifecycleListener(LifecycleListener *lifecycleListener);

            /**
            * Shuts down this HazelcastClient.
            */
            void shutdown();

            internal::nearcache::NearCacheManager &getNearCacheManager();

            serialization::pimpl::SerializationService &getSerializationService();
        private:
            boost::shared_ptr<spi::ClientProxy> getDistributedObjectForService(const std::string &serviceName,
                                                                               const std::string &name,
                                                                               spi::ClientProxyFactory &factory);

            ClientConfig clientConfig;
            ClientProperties clientProperties;
            util::CountDownLatch shutdownLatch;
            spi::ClientContext clientContext;
            spi::LifecycleService lifecycleService;
            serialization::pimpl::SerializationService serializationService;
            std::auto_ptr<connection::ConnectionManager> connectionManager;
            internal::nearcache::NearCacheManager nearCacheManager;
            spi::ClusterService clusterService;
            spi::PartitionService partitionService;
            spi::InvocationService invocationService;
            spi::ServerListenerService serverListenerService;
            Cluster cluster;
            spi::ProxyManager proxyManager;

            HazelcastClient(const HazelcastClient& rhs);

            void operator=(const HazelcastClient& rhs);

            const std::string TOPIC_RB_PREFIX;
        };

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT */
