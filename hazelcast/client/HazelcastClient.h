#ifndef HAZELCAST_CLIENT
#define HAZELCAST_CLIENT

#include "serialization/SerializationService.h"
#include "ClientConfig.h"
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

        class IdGenerator;

        class IAtomicLong;

        class ICountDownLatch;

        class ISemaphore;
        //TODO  Lock , Topic

        class HazelcastClient {
            friend class ClusterService;

        public:
            HazelcastClient(ClientConfig&);

            template<typename K, typename V>
            IMap<K, V> getMap(std::string instanceName) {
                return IMap<K, V >(instanceName);
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

            ~HazelcastClient();

            serialization::SerializationService& getSerializationService();

            ClientConfig& getClientConfig();

            connection::ConnectionManager& getConnectionManager();

        private:
            class HazelcastClientImpl;

            HazelcastClientImpl *impl;


            HazelcastClient(const HazelcastClient& rhs);


        };

    }
}
#endif /* HAZELCAST_CLIENT */