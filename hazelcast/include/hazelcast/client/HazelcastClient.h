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
/*! \mainpage My Personal Index Page
 *
 * \section intro_sec Introduction
 *
 * This is the introduction.
 *
 * \section install_sec Installation
 *
 * \subsection step1 Step 1: Opening the box
 *
 * etc...
 *
 */
namespace hazelcast {
    namespace client {
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