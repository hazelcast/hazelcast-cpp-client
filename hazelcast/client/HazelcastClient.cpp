#include "HazelcastClient.h"
#include "IdGenerator.h"
#include "IAtomicLong.h"
#include "ICountDownLatch.h"
#include "ISemaphore.h"
#include "connection/ConnectionManager.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient::HazelcastClientImpl {
        public:
            HazelcastClientImpl(ClientConfig& clientConfig)
            : clientConfig(clientConfig)
            , serializationService(0)
            , connectionManager(serializationService, clientConfig) {

            };

            serialization::SerializationService& getSerializationService() {
                return serializationService;
            };

            ClientConfig& getClientConfig() {
                return clientConfig;
            };

            connection::ConnectionManager& getConnectionManager() {
                return connectionManager;
            };

            ClientConfig clientConfig;
            serialization::SerializationService serializationService;
            connection::ConnectionManager connectionManager;
        };

        HazelcastClient::HazelcastClient(ClientConfig& config)
        :impl(new HazelcastClientImpl(config)) {
        };


        HazelcastClient::HazelcastClient(HazelcastClient const & rhs) {

        }

        HazelcastClient::~HazelcastClient() {
            delete impl;
        }

        serialization::SerializationService& HazelcastClient::getSerializationService() {
            return impl->getSerializationService();
        }


        ClientConfig& HazelcastClient::getClientConfig() {
            return impl->getClientConfig();
        };

        connection::ConnectionManager & HazelcastClient::getConnectionManager() {
            return impl->getConnectionManager();
        }

        IdGenerator HazelcastClient::getIdGenerator(std::string instanceName) {
            return IdGenerator(instanceName);
        };

        IAtomicLong HazelcastClient::getIAtomicLong(std::string instanceName) {
            return IAtomicLong(instanceName);
        };

        ICountDownLatch HazelcastClient::getICountDownLatch(std::string instanceName) {
            return ICountDownLatch(instanceName);
        };

        ISemaphore HazelcastClient::getISemaphore(std::string instanceName) {
            return ISemaphore(instanceName);
        };
    }
}