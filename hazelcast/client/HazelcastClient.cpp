#include "IdGenerator.h"
#include "IAtomicLong.h"
#include "ICountDownLatch.h"
#include "spi/PartitionService.h"
#include "ISemaphore.h"
#include "Cluster.h"
#include "ClientConfig.h"
#include "HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient::HazelcastClientImpl {

        public:
            HazelcastClientImpl(ClientConfig& clientConfig, HazelcastClient *client)
            : clientConfig(clientConfig)
            , serializationService(0)
            , clientContext(*client)
            , clusterService(clientContext)
            , connectionManager(clusterService, serializationService, clientConfig)
            , partitionService(clusterService, serializationService)
            , invocationService(clusterService, partitionService)
            , cluster(clusterService) {
                LoadBalancer *loadBalancer = this->clientConfig.getLoadBalancer();
                loadBalancer->init(cluster);
            };

            void shutdown() {
                //TODO
            };

            ClientConfig clientConfig;
            serialization::SerializationService serializationService;
            spi::ClientContext clientContext;
            spi::ClusterService clusterService;
            connection::ConnectionManager connectionManager;
            spi::PartitionService partitionService;
            spi::InvocationService invocationService;
            Cluster cluster;

        };

        HazelcastClient::HazelcastClient(ClientConfig& config)
        :impl(new HazelcastClientImpl(config, this)) {
            impl->clusterService.start();
            impl->partitionService.start();

        };

        HazelcastClient::HazelcastClient(HazelcastClient const & rhs) {

        };

        HazelcastClient::~HazelcastClient() {
            delete impl;
        };

        serialization::SerializationService& HazelcastClient::getSerializationService() {
            return impl->serializationService;
        };


        ClientConfig& HazelcastClient::getClientConfig() {
            return impl->clientConfig;
        };

        connection::ConnectionManager & HazelcastClient::getConnectionManager() {
            return impl->connectionManager;
        };

        spi::InvocationService & HazelcastClient::getInvocationService() {
            return impl->invocationService;
        };

        spi::ClusterService & HazelcastClient::getClusterService() {
            return impl->clusterService;
        };


        spi::PartitionService & HazelcastClient::getPartitionService() {
            return impl->partitionService;
        };

        spi::ClientContext & HazelcastClient::getClientContext() {
            return impl->clientContext;
        };

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

        void HazelcastClient::shutdown() {
            impl->shutdown();
        };


    }
}