#include "HazelcastClient.h"
#include "IdGenerator.h"
#include "IAtomicLong.h"
#include "ICountDownLatch.h"
#include "ISemaphore.h"
#include "ClientConfig.h"
#include "Cluster.h"

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
            , clientContext(*client)
            , cluster(clusterService) {
                LoadBalancer *loadBalancer = this->clientConfig.getLoadBalancer();
                loadBalancer->init(cluster);
            };

            ClientConfig clientConfig;
            Cluster cluster;
            serialization::SerializationService serializationService;
            connection::ConnectionManager connectionManager;
            spi::ClusterService clusterService;
            spi::InvocationService invocationService;
            spi::ClientContext clientContext;

        };

        HazelcastClient::HazelcastClient(ClientConfig& config)
        :impl(new HazelcastClientImpl(config, this)) {
            impl->clusterService.start();

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