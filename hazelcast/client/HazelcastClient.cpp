#include "IdGenerator.h"
#include "IAtomicLong.h"
#include "ICountDownLatch.h"
#include "ISemaphore.h"
#include "ClientConfig.h"
#include "Cluster.h"
#include "HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient::HazelcastClientImpl {

        public:
            HazelcastClientImpl(ClientConfig& clientConfig, HazelcastClient& client)
            : clientConfig(clientConfig)
            , lifecycleService(client, clientConfig)
            , serializationService(0)
            , clientContext(client)
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
            spi::LifecycleService lifecycleService;
            serialization::SerializationService serializationService;
            spi::ClientContext clientContext;
            spi::ClusterService clusterService;
            connection::ConnectionManager connectionManager;
            spi::PartitionService partitionService;
            spi::InvocationService invocationService;
            Cluster cluster;

        };

        HazelcastClient::HazelcastClient(ClientConfig& config)
        :impl(new HazelcastClientImpl(config, *this)) {
            impl->clusterService.start();
            impl->partitionService.start();
            impl->lifecycleService.setStarted();

        };

        HazelcastClient::HazelcastClient(HazelcastClient const & rhs) {
            //private;
        };


        void HazelcastClient::operator = (const HazelcastClient& rhs) {
            //private
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

        spi::LifecycleService & HazelcastClient::getLifecycleService() {
            return impl->lifecycleService;
        };

        spi::PartitionService & HazelcastClient::getPartitionService() {
            return impl->partitionService;
        };

        spi::ClientContext & HazelcastClient::getClientContext() {
            return impl->clientContext;
        };

        IdGenerator HazelcastClient::getIdGenerator(const std::string& instanceName) {
            return getDistributedObject< IdGenerator >(instanceName);
        };

        IAtomicLong HazelcastClient::getIAtomicLong(const std::string& instanceName) {
            return getDistributedObject< IAtomicLong >(instanceName);
        };

        ICountDownLatch HazelcastClient::getICountDownLatch(const std::string& instanceName) {
            return getDistributedObject< ICountDownLatch >(instanceName);
        };

        ISemaphore HazelcastClient::getISemaphore(const std::string& instanceName) {
            return getDistributedObject< ISemaphore >(instanceName);
        };

        void HazelcastClient::shutdown() {
            impl->shutdown();
        };


    }
}