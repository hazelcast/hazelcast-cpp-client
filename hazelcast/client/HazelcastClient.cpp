#include "HazelcastClient.h"
#include "IdGenerator.h"
#include "IAtomicLong.h"
#include "ICountDownLatch.h"
#include "ISemaphore.h"
#include "ClientConfig.h"
#include "connection/ConnectionManager.h"
#include "./spi/ClusterService.h"
#include "InvocationService.h"


namespace hazelcast {
    namespace client {

        class HazelcastClient::HazelcastClientImpl {

        public:
            HazelcastClientImpl(ClientConfig& clientConfig, HazelcastClient *client)
            : clientConfig(clientConfig)
            , serializationService(0)
            , connectionManager(serializationService, clientConfig)
            , clusterService(*client)
            , invocationService(clusterService)
            , clientContext(*client) {
                clusterService.start();
            };

            ClientConfig clientConfig;
            serialization::SerializationService serializationService;
            connection::ConnectionManager connectionManager;
            spi::ClusterService clusterService;
            spi::InvocationService invocationService;
            spi::ClientContext clientContext;
        };

        HazelcastClient::HazelcastClient(ClientConfig& config)
        :impl(new HazelcastClientImpl(config, this)) {
        };

        HazelcastClient::HazelcastClient(HazelcastClient const & rhs) {

        }

        HazelcastClient::~HazelcastClient() {
            delete impl;
        }

        serialization::SerializationService& HazelcastClient::getSerializationService() {
            return impl->serializationService;
        }


        ClientConfig& HazelcastClient::getClientConfig() {
            return impl->clientConfig;
        };

        connection::ConnectionManager & HazelcastClient::getConnectionManager() {
            return impl->connectionManager;
        }

        spi::InvocationService & HazelcastClient::getInvocationService() {
            return impl->invocationService;
        }

        spi::ClusterService & HazelcastClient::getClusterService() {
            return impl->clusterService;
        }


        spi::ClientContext & HazelcastClient::getClientContext() {
            return impl->clientContext;
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