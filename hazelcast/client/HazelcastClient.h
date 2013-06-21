#ifndef HAZELCAST_CLIENT
#define HAZELCAST_CLIENT

#include "IMap.h"
#include "MultiMap.h"
#include "IQueue.h"
#include "ISet.h"
#include "IList.h"
#include "ITopic.h"

#include <memory>
#include <map>

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
        }

        class ClientConfig;

        class IdGenerator;

        class IAtomicLong;

        class ICountDownLatch;

        class ISemaphore;
        //TODO  Lock

        class HazelcastClient {
            friend class hazelcast::client::spi::ClusterService;

            friend class hazelcast::client::spi::ClientContext;

        public:
            HazelcastClient(ClientConfig&);

            ~HazelcastClient();

            template <typename T>
            T getDistributedObject(const std::string& instanceName) {
                return T(instanceName, getClientContext());
            };

            template<typename K, typename V>
            IMap<K, V> getMap(const std::string& instanceName) {
                return getDistributedObject< IMap<K, V > >(instanceName);
            };

            template<typename K, typename V>
            MultiMap<K, V> getMultiMap(const std::string& instanceName) {
                return getDistributedObject< MultiMap<K, V > >(instanceName);
            };

            template<typename E>
            IQueue<E> getQueue(const std::string& instanceName) {
                return getDistributedObject< IQueue<E > >(instanceName);
            };

            template<typename E>
            ISet<E> getSet(const std::string& instanceName) {
                return getDistributedObject< ISet<E > >(instanceName);
            };

            template<typename E>
            IList<E> getList(const std::string& instanceName) {
                return getDistributedObject< IList<E > >(instanceName);
            };

            template<typename E>
            ITopic<E> getTopic(const std::string& instanceName) {
                return getDistributedObject< ITopic<E> >(instanceName);
            };

            IdGenerator getIdGenerator(const std::string& instanceName);

            IAtomicLong getIAtomicLong(const std::string& instanceName);

            ICountDownLatch getICountDownLatch(const std::string& instanceName);


            ISemaphore getISemaphore(const std::string& instanceName);


            ClientConfig& getClientConfig();

            void shutdown();

        private:
            class HazelcastClientImpl;

            HazelcastClientImpl *impl;

            spi::ClientContext& getClientContext();

            connection::ConnectionManager& getConnectionManager();

            serialization::SerializationService& getSerializationService();

            spi::InvocationService& getInvocationService();

            spi::ClusterService& getClusterService();

            spi::PartitionService& getPartitionService();

            spi::LifecycleService & getLifecycleService();

            HazelcastClient(const HazelcastClient& rhs);

            void operator = (const HazelcastClient& rhs);


        };

    }
}
#endif /* HAZELCAST_CLIENT */