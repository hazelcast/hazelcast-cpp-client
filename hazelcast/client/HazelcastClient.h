#ifndef HAZELCAST_CLIENT
#define HAZELCAST_CLIENT

#include "IMap.h"
#include "MultiMap.h"
#include "IQueue.h"
#include "ISet.h"
#include "IList.h"

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
        }

        class ClientConfig;

        class IdGenerator;

        class IAtomicLong;

        class ICountDownLatch;

        class ISemaphore;
        //TODO  Lock , Topic

        class HazelcastClient {
            friend class hazelcast::client::spi::ClusterService;

            friend class hazelcast::client::spi::ClientContext;

        public:
            HazelcastClient(ClientConfig&);

            ~HazelcastClient();

            template<typename K, typename V>
            IMap<K, V> getMap(std::string instanceName) {
                return IMap<K, V >(instanceName, getClientContext());
            };

            template<typename K, typename V>
            MultiMap<K, V> getMultiMap(std::string instanceName) {
                return MultiMap<K, V >(instanceName);
            };

            template<typename E>
            IQueue<E> getQueue(std::string instanceName) {
                return IQueue<E >(instanceName);
            };

            template<typename E>
            ISet<E> getSet(std::string instanceName) {
                return ISet<E >(instanceName);
            };

            template<typename E>
            IList<E> getList(std::string instanceName) {
                return IList<E >(instanceName);
            };

            IdGenerator getIdGenerator(std::string instanceName);

            IAtomicLong getIAtomicLong(std::string instanceName);

            ICountDownLatch getICountDownLatch(std::string instanceName);

            ISemaphore getISemaphore(std::string instanceName);


            ClientConfig& getClientConfig();

        private:
            class HazelcastClientImpl;

            HazelcastClientImpl *impl;

            spi::ClientContext& getClientContext();

            connection::ConnectionManager& getConnectionManager();

            serialization::SerializationService& getSerializationService();

            spi::InvocationService& getInvocationService();

            spi::ClusterService& getClusterService();

            HazelcastClient(const HazelcastClient& rhs);


        };

    }
}
#endif /* HAZELCAST_CLIENT */