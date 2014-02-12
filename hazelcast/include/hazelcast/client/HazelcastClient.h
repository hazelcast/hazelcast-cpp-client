#ifndef HAZELCAST_CLIENT
#define HAZELCAST_CLIENT

#include "hazelcast/client/IMap.h"
#include "hazelcast/client/MultiMap.h"
#include "hazelcast/client/IQueue.h"
#include "hazelcast/client/ISet.h"
#include "hazelcast/client/IList.h"
#include "hazelcast/client/ITopic.h"
#include "hazelcast/client/TransactionOptions.h"
#include "hazelcast/client/TransactionContext.h"
#include "hazelcast/client/TransactionalTaskContext.h"

namespace hazelcast {
    namespace client {
/*! \mainpage Introduction
 *
 * You can use native C++ Client to connect to hazelcast nodes and make almost all operations that a node does.
 * Different from nodes, clients do not hold data.
 *
 * Some of features of C++ Clients are:
 * * Access to distributed data structures like IMap, IQueue, MultiMap, ITopic etc... @see DistributedObject
 * * Access to transactional distributed data structures like TransactionalMap, TransactionalQueue etc...
 * * Ability to add cluster listeners to a cluster and entry/item listeners to distributed data structures.
 * @see MembershipListener, IMap#addEntryListener , IQueue#addItemListener etc .
 * * Distributed synchronization mechanisms with ILock , ISemaphore and ICountDownLatch.
 * * C++ Client is smart by default, which means that it knows where the data is and asks directly to correct node.
 * Note that you can turn this feature off ( ClientConfig#setSmart), if you don't want your clients to connect every
 * node.
 *
 *
 * Features that are planned to implement are:
 * * Distributed Executor Service
 * * NearCache to store some of the data in client side to reduce latencies.
 * * Access to replicated Map
 * * SSL Socket support
 * * Access to AtomicReference
 * * Condition related methods to ILock.
 *
 *
 * \section how_to_setup How to Setup
 *
 * Hazelcast C++ client is shipped with 32 & 64 bit, shared and static libraries. Compiled static libraries of dependencies
 * are also available in the release.
 *
 * Dependencies are zlib and some of the boost libraries. Needed boost libraries are:
 *  * libboost_atomic
 *  * libboost_system,
 *  * libboost_chrono,
 *  * libboost_thread and
 *  * libboost_date_time.
 *
 *  Downloaded release folder consist of
 *  *  docs/ html doxygen documentations are here.
 *  *  hazelcast/
 *      +  lib/ Contains both shared and static library of hazelcast.
 *      +  include/ Contains headers of client
 *  * external/
 *      + lib/     Contains compiled static libraries of dependencies.
 *      + include/ Contains headers of dependencies
 *
 *
 * \section guides Platform Specific Installation Guides
 *
 * C++ client is tested on Linux 32/64, Mac 64 and Windows 32/64 bit machines.
 *
 * \subsection Linux
 *
 * For linux, there are two distributions one is 32bit the other is 64bit.
 *
 * Sample script to build with static library:
 *
 *  * ﻿g++ main.cpp -pthread -I./external/include -I./hazelcast/include ./hazelcast/lib/libHazelcastClientStatic_64.a ./external/lib/libz.a ./external/lib/libboost_thread.a ./external/lib/libboost_system.a ./external/lib/libboost_date_time.a ./external/lib/libboost_chrono.a ./external/libboost_atomic.a
 *
 * Sample script to build with shared library:
 *
 *  * g++ main.cpp -lpthread -Wl,--no-as-needed -lrt -I./external/include -I./hazelcast/include -L./hazelcast/lib -lHazelcastClientShared_64 ./external/lib/libz.a ./external/lib/libboost_thread.a ./external/lib/libboost_system.a ./external/lib/libboost_date_time.a ./external/lib/libboost_chrono.a ./external/lib/libboost_atomic.a
 *
 * \subsection Mac
 *
 * For Mac, there is only one  distribution which is 64 bit.
 *
 * Sample script to build with static library:
 *
 *  * g++ main.cpp -I./external/include -I./hazelcast/include ./hazelcast/lib/libHazelcastClientStatic_64.a ./external/lib/darwin/libz.a ./external/lib/darwin/libboost_thread.a ./external/lib/darwin/libboost_system.a ./external/lib/darwin/libboost_exception.a ./external/lib/darwin/libboost_date_time.a ./external/lib/darwin/libboost_chrono.a ./external/lib/darwin/libboost_atomic.a
 *
 * Sample script to build with shared library:
 *
 *  * g++ main.cpp -I./external/include -I./hazelcast/include -L./hazelcast/lib -lHazelcastClientShared_64 ./external/lib/darwin/libz.a ./external/lib/darwin/libboost_thread.a ./external/lib/darwin/libboost_system.a ./external/lib/darwin/libboost_exception.a ./external/lib/darwin/libboost_date_time.a ./external/lib/darwin/libboost_chrono.a ./external/lib/darwin/libboost_atomic.a
 *
 * \subsection Windows
 *
 * For Windows, there are two distributions one is 32bit the other is 64bit.
 * 
 * 
 * 
 * \section code_samples Code Samples
 *
 * Note that these codes to work, there should be a Hazelcast node is running.
 *
 * \subsection map Map example
 *
 *          #include <hazelcast/client/HazelcastAll.h>
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
 *              return 0;
 *          }
 *
 * \subsection queue Queue Example
 *
 *          #include <hazelcast/client/HazelcastAll.h>
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
 *      #include "hazelcast/client/ClientConfig.h"
 *      #include "hazelcast/client/EntryEvent.h"
 *      #include "hazelcast/client/IMap.h"
 *      #include "hazelcast/client/Address.h"
 *      #include "hazelcast/client/HazelcastClient.h"
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
                writer.writeUTF("n", name);
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
        class Foo : public Portable {
        public:
            int getFactoryId() const{
                return 666;
            };

            int getClassId() const{
                return 2;
            };

            void writePortable(serialization::PortableWriter &writer) const{
                writer.writeUTF("n", name);
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

        class Bar : public IdentifiedDataSerializable {
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
 *  or as an Item in IQueue
 *
 *
 * .
 */
        namespace connection {
            class ConnectionManager;
        }

        namespace serialization {
            class SerializationService;
        }
        namespace spi {
            class ClientContext;

            class InvocationService;

            class ClusterService;

            class PartitionService;

            class LifecycleService;

            class ServerListenerService;

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
             */
            HazelcastClient(ClientConfig &);

            /**
             * Destructor
             */
            ~HazelcastClient();

            /**
             *
             * @param T type of the distributed object
             * @param instanceName name of the distributed object.
             * @returns distributed object
             */
            template <typename T>
            T getDistributedObject(const std::string &instanceName) {
                T t(instanceName, &getClientContext());
                return t;
            };

            /**
             * Returns the distributed map instance with the specified name.
             *
             * @param K key type
             * @param V value type
             * @param instanceName name of the distributed map
             * @return distributed map instance with the specified name
             */
            template<typename K, typename V>
            IMap<K, V> getMap(const std::string &instanceName) {
                return getDistributedObject< IMap<K, V > >(instanceName);
            };

            /**
             * Returns the distributed multimap instance with the specified name.
             *
             * @param name name of the distributed multimap
             * @return distributed multimap instance with the specified name
             */
            template<typename K, typename V>
            MultiMap<K, V> getMultiMap(const std::string &instanceName) {
                return getDistributedObject< MultiMap<K, V > >(instanceName);
            };

            /**
             * Returns the distributed queue instance with the specified name and entry type E.
             *
             * @param name name of the distributed queue
             * @return distributed queue instance with the specified name
             */
            template<typename E>
            IQueue<E> getQueue(const std::string &instanceName) {
                return getDistributedObject< IQueue<E > >(instanceName);
            };

            /**
             * Returns the distributed set instance with the specified name and entry type E.
             * Set is ordered unique set of entries. similar to std::set
             *
             * @param name name of the distributed set
             * @return distributed set instance with the specified name
             */

            template<typename E>
            ISet<E> getSet(const std::string &instanceName) {
                return getDistributedObject< ISet<E > >(instanceName);
            };

            /**
             * Returns the distributed list instance with the specified name.
             * List is ordered set of entries. similar to std::vector
             *
             * @param name name of the distributed list
             * @return distributed list instance with the specified name
             */
            template<typename E>
            IList<E> getList(const std::string &instanceName) {
                return getDistributedObject< IList<E > >(instanceName);
            };

            /**
             * Returns the distributed topic instance with the specified name and entry type E.
             *
             * @param name name of the distributed topic
             * @return distributed topic instance with the specified name
             */
            template<typename E>
            ITopic<E> getTopic(const std::string &instanceName) {
                return getDistributedObject< ITopic<E> >(instanceName);
            };

            /**
            * Creates cluster-wide unique IDs. Generated IDs are long type primitive values
            * between <tt>0</tt> and <tt>LONG_MAX</tt> . Generated IDs are unique during the life
            * cycle of the cluster. If the entire cluster is restarted, IDs start from <tt>0</tt> again.
            *
            * @param name name of the IdGenerator
            * @return IdGenerator for the given name
            */
            IdGenerator getIdGenerator(const std::string &instanceName);

            /**
            * Creates cluster-wide atomic long. Hazelcast IAtomicLong is distributed
            * implementation of <tt>java.util.concurrent.atomic.AtomicLong</tt>.
            *
            * @param name name of the IAtomicLong proxy
            * @return IAtomicLong proxy for the given name
            */
            IAtomicLong getIAtomicLong(const std::string &instanceName);

            /**
             * Creates cluster-wide CountDownLatch. Hazelcast ICountDownLatch is distributed
             * implementation of <tt>java.util.concurrent.CountDownLatch</tt>.
             *
             * @param name name of the ICountDownLatch proxy
             * @return ICountDownLatch proxy for the given name
             */

            ICountDownLatch getICountDownLatch(const std::string &instanceName);

            /**
             * Returns the distributed lock instance for the specified key object.
             * The specified object is considered to be the key for this lock.
             * So keys are considered equals cluster-wide as long as
             * they are serialized to the same byte array such as String, long,
             * Integer.
             * <p/>
             * Locks are fail-safe. If a member holds a lock and some of the
             * members go down, cluster will keep your locks safe and available.
             * Moreover, when a member leaves the cluster, all the locks acquired
             * by this dead member will be removed so that these locks can be
             * available for live members immediately.
             * <pre>
             * Lock lock = hazelcastInstance.getLock("PROCESS_LOCK");
             * lock.lock();
             * try {
             *   // process
             * } finally {
             *   lock.unlock();
             * }
             * </pre>
             *
             * @param key key of the lock instance
             * @return distributed lock instance for the specified key.
             */
            ILock getILock(const std::string &instanceName);

            /**
             * Creates cluster-wide semaphore. Hazelcast ISemaphore is distributed
             * implementation of <tt>java.util.concurrent.Semaphore</tt>.
             *
             * @param name name of the ISemaphore proxy
             * @return ISemaphore proxy for the given name
             */
            ISemaphore getISemaphore(const std::string &instanceName);

            /**
             *
             * @return configuration of this Hazelcast client.
             */
            ClientConfig &getClientConfig();

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
            TransactionContext newTransactionContext(const TransactionOptions &options);

            /**
             * Executes given transactional task in current thread using default options
             * and returns the result of the task.
             *
             * @param task task to be executed
             * @param T return type of task
             * @param TransactionalTask transactional task should implement "T execute(TransactionalTaskContext &context)"
             * @return returns result of transactional task
             *
             * @throws TransactionException if an error occurs during transaction.
             */
            template<typename T, typename TransactionalTask >
            T executeTransaction(const TransactionalTask &task) {
                TransactionOptions defaultOptions;
                return executeTransaction<T, TransactionalTask>(defaultOptions, task);
            };

            /**
             * Executes given transactional task in current thread using given options
             * and returns the result of the task.
             *
             * @param options options for this transactional task
             * @param task task to be executed
             * @param <T> return type of task
             * @return returns result of transactional task
             *
             * @throws TransactionException if an error occurs during transaction.
             */
            template<typename T, typename TransactionalTask >
            T executeTransaction(const TransactionOptions &options, const TransactionalTask &task) {
                TransactionContext context = newTransactionContext(options);
                TransactionalTaskContext transactionalTaskContext(context);
                context.beginTransaction();
                try {
                    T value = task.execute(transactionalTaskContext);
                    context.commitTransaction();
                    return value;
                } catch (std::exception &e) {
                    context.rollbackTransaction();
                    throw e;
                }
            };

            /**
             * Returns the Cluster that this Hazelcast instance is part of.
             * Cluster interface allows you to add listener for membership
             * events and learn more about the cluster that this Hazelcast
             * instance is part of.
             *
             * @return cluster that this Hazelcast instance is part of
             */
            Cluster &getCluster();
        private:

            class HazelcastClientImpl;

            HazelcastClientImpl *impl;

            spi::ClientContext &getClientContext();

            connection::ConnectionManager &getConnectionManager();

            serialization::SerializationService &getSerializationService();

            spi::InvocationService &getInvocationService();

            spi::ClusterService &getClusterService();

            spi::PartitionService &getPartitionService();

            spi::LifecycleService &getLifecycleService();

            spi::ServerListenerService &getServerListenerService();

            HazelcastClient(const HazelcastClient &rhs);

            void operator = (const HazelcastClient &rhs);

        };

    }
}
#endif /* HAZELCAST_CLIENT */