#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IdGenerator.h"
#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/ILock.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/connection/ConnectionManager.h"

namespace hazelcast {
    namespace client {

        /**
         * Private API.
         */
        class HazelcastClient::HazelcastClientImpl {
        public:
            HazelcastClientImpl(ClientConfig &clientConfig, HazelcastClient &hazelcastClient)
            : clientConfig(clientConfig)
            , clientContext(hazelcastClient)
            , lifecycleService(clientContext, clientConfig)
            , serializationService(0)
            , connectionManager(clientContext, this->clientConfig.isSmart())
            , clusterService(clientContext)
            , partitionService(clientContext)
            , invocationService(clientContext)
            , serverListenerService(clientContext)
            , cluster(clusterService) {
                LoadBalancer *loadBalancer = this->clientConfig.getLoadBalancer();
                loadBalancer->init(cluster);
            };


            ~HazelcastClientImpl() {
                lifecycleService.shutdown();
            }

            ClientConfig clientConfig;
            spi::ClientContext clientContext;
            spi::LifecycleService lifecycleService;
            serialization::SerializationService serializationService;
            connection::ConnectionManager connectionManager;
            spi::ClusterService clusterService;
            spi::PartitionService partitionService;
            spi::InvocationService invocationService;
            spi::ServerListenerService serverListenerService;
            Cluster cluster;

        };

        HazelcastClient::HazelcastClient(ClientConfig &config)
        :impl(new HazelcastClientImpl(config, *this)) {
            srand(time(NULL));
            impl->lifecycleService.start();
            impl->connectionManager.start();
            impl->invocationService.start();
            impl->clusterService.start();
            impl->partitionService.start();

        };

        HazelcastClient::HazelcastClient(HazelcastClient const &rhs) {
            //private;
        };

        void HazelcastClient::operator = (const HazelcastClient &rhs) {
            //private
        };

        HazelcastClient::~HazelcastClient() {
            delete impl;
        };

        serialization::SerializationService &HazelcastClient::getSerializationService() {
            return impl->serializationService;
        };

        ClientConfig &HazelcastClient::getClientConfig() {
            return impl->clientConfig;
        };

        spi::ClientContext &HazelcastClient::getClientContext() {
            return impl->clientContext;
        };

        Cluster &HazelcastClient::getCluster() {
            return impl->cluster;
        }

        spi::InvocationService &HazelcastClient::getInvocationService() {
            return impl->invocationService;
        };

        spi::ServerListenerService &HazelcastClient::getServerListenerService() {
            return impl->serverListenerService;
        };

        spi::ClusterService &HazelcastClient::getClusterService() {
            return impl->clusterService;
        };

        spi::PartitionService &HazelcastClient::getPartitionService() {
            return impl->partitionService;
        };

        spi::LifecycleService &HazelcastClient::getLifecycleService() {
            return impl->lifecycleService;
        };

        connection::ConnectionManager &HazelcastClient::getConnectionManager() {
            return impl->connectionManager;
        };

        IdGenerator HazelcastClient::getIdGenerator(const std::string &instanceName) {
            return getDistributedObject< IdGenerator >(instanceName);
        };

        IAtomicLong HazelcastClient::getIAtomicLong(const std::string &instanceName) {
            return getDistributedObject< IAtomicLong >(instanceName);
        };

        ICountDownLatch HazelcastClient::getICountDownLatch(const std::string &instanceName) {
            return getDistributedObject< ICountDownLatch >(instanceName);
        };

        ISemaphore HazelcastClient::getISemaphore(const std::string &instanceName) {
            return getDistributedObject< ISemaphore >(instanceName);
        };

        ILock HazelcastClient::getILock(const std::string &instanceName) {
            return getDistributedObject< ILock >(instanceName);
        };

        TransactionContext HazelcastClient::newTransactionContext() {
            TransactionOptions defaultOptions;
            return newTransactionContext(defaultOptions);
        }

        TransactionContext HazelcastClient::newTransactionContext(const TransactionOptions &options) {
            return TransactionContext(impl->clientContext, options);
        }

    }
}