#ifndef HAZELCAST_CLIENT
#define HAZELCAST_CLIENT

#include "protocol/CommandHandler.h"
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

        class IdGenerator;
        class IAtomicLong;
        class ICountDownLatch;
        class ISemaphore;
        //TODO  Lock , Topic

        class HazelcastClient {
            friend class ClientService;

        public:
            static std::auto_ptr<HazelcastClient> newHazelcastClient(ClientConfig& config);

            template<typename K, typename V>
            IMap<K, V> getMap(std::string instanceName) {
                return IMap<K, V > (instanceName, clientService);
            };

            template<typename K, typename V>
            MultiMap<K, V> getMultiMap(std::string instanceName) {
                return MultiMap<K, V > (instanceName, clientService);
            };

            template<typename E>
            IQueue<E> getQueue(std::string instanceName) {
                return IQueue<E > (instanceName, clientService);
            };

            template<typename E>
            ISet<E> getSet(std::string instanceName) {
                return ISet<E > (instanceName, clientService);
            };

            template<typename E>
            IList<E> getList(std::string instanceName) {
                return IList<E > (instanceName, clientService);
            };

            IdGenerator getIdGenerator(std::string instanceName);
            IAtomicLong getIAtomicLong(std::string instanceName);
            ICountDownLatch getICountDownLatch(std::string instanceName);
            ISemaphore getISemaphore(std::string instanceName);
            ~HazelcastClient();

        private:
            serialization::SerializationService& getSerializationService();
            protocol::CommandHandler& getCommandHandler();
            ClientConfig& getClientConfig();

            void setupInitialConnection();

            HazelcastClient(ClientConfig&);
            HazelcastClient(const HazelcastClient&);

            ClientService clientService;
            ClientConfig clientConfig;
            protocol::CommandHandler commandHandler;
            serialization::SerializationService serializationService;
        };

    }
}
#endif /* HAZELCAST_CLIENT */