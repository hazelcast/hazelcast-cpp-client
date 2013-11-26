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

        class IExecutorService;

        class TransactionContext;

        class TransactionOptions;

        class Cluster;

        class HazelcastClient {
            friend class spi::ClientContext;

            friend class spi::LifecycleService;

        public:
            HazelcastClient(ClientConfig &);

            ~HazelcastClient();

            template <typename T>
            T getDistributedObject(const std::string &instanceName) {
                T t(instanceName, &getClientContext());
                return t;
            };

            template<typename K, typename V>
            IMap<K, V> getMap(const std::string &instanceName) {
                return getDistributedObject< IMap<K, V > >(instanceName);
            };

            template<typename K, typename V>
            MultiMap<K, V> getMultiMap(const std::string &instanceName) {
                return getDistributedObject< MultiMap<K, V > >(instanceName);
            };

            template<typename E>
            IQueue<E> getQueue(const std::string &instanceName) {
                return getDistributedObject< IQueue<E > >(instanceName);
            };

            template<typename E>
            ISet<E> getSet(const std::string &instanceName) {
                return getDistributedObject< ISet<E > >(instanceName);
            };

            template<typename E>
            IList<E> getList(const std::string &instanceName) {
                return getDistributedObject< IList<E > >(instanceName);
            };

            template<typename E>
            ITopic<E> getTopic(const std::string &instanceName) {
                return getDistributedObject< ITopic<E> >(instanceName);
            };

            IdGenerator getIdGenerator(const std::string &instanceName);

            IAtomicLong getIAtomicLong(const std::string &instanceName);

            ICountDownLatch getICountDownLatch(const std::string &instanceName);

            ILock getILock(const std::string &instanceName);

            IExecutorService getExecutorService(const std::string &instanceName);

            ISemaphore getISemaphore(const std::string &instanceName);

            ClientConfig &getClientConfig();

            TransactionContext newTransactionContext();

            TransactionContext newTransactionContext(const TransactionOptions &options);

            template<typename T, typename TransactionalTask > T executeTransaction(const TransactionalTask &task) {

                TransactionOptions defaultOptions;
                return executeTransaction<T, TransactionalTask>(defaultOptions, task);
            };

            template<typename T, typename TransactionalTask >
            T executeTransaction(const TransactionOptions &options,  const TransactionalTask &task) {
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


        private:
            class HazelcastClientImpl;

            HazelcastClientImpl *impl;

            spi::ClientContext &getClientContext();

            Cluster &getCluster();

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