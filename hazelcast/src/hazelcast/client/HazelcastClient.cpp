#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IdGenerator.h"
#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/ILock.h"
#include "hazelcast/client/IExecutorService.h"
#include "hazelcast/client/connection/SmartConnectionManager.h"
#include "hazelcast/client/connection/DummyConnectionManager.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient::HazelcastClientImpl {
        public:
            HazelcastClientImpl(ClientConfig& clientConfig, HazelcastClient& client)
            : clientConfig(clientConfig)
            , lifecycleService(client, this->clientConfig)
            , serializationService(0)
            , connectionManager(clientConfig.isSmart() ? (connection::ConnectionManager *) new connection::SmartConnectionManager(clusterService, serializationService, this->clientConfig)
                    : (connection::ConnectionManager *) new connection::DummyConnectionManager(clusterService, serializationService, this->clientConfig))
            , clusterService(partitionService, lifecycleService, *connectionManager, serializationService, this->clientConfig)
            , partitionService(clusterService, serializationService)
            , invocationService(clusterService, partitionService)
            , serverListenerService(invocationService)
            , cluster(clusterService)
            , clientContext(client) {
                LoadBalancer *loadBalancer = this->clientConfig.getLoadBalancer();
                loadBalancer->init(cluster);
            };


            ~HazelcastClientImpl() {
                //TODO check if already shutdown
                //TODO shutdown the threads ????
                lifecycleService.setShutdown();
            }

            ClientConfig clientConfig;
            spi::LifecycleService lifecycleService;
            serialization::SerializationService serializationService;
            std::auto_ptr< connection::ConnectionManager> connectionManager;
            spi::ClusterService clusterService;
            spi::PartitionService partitionService;
            spi::InvocationService invocationService;
            spi::ServerListenerService serverListenerService;
            spi::DistributedObjectListenerService distributedObjectListenerService;
            Cluster cluster;
            spi::ClientContext clientContext;

        };

        HazelcastClient::HazelcastClient(ClientConfig& config)
        :impl(new HazelcastClientImpl(config, *this)) {
            srand(time(NULL));
            impl->lifecycleService.setStarted();
            impl->clusterService.start();
            impl->partitionService.start();

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

        void HazelcastClient::triggerDistributedObjectAdded(const std::string& name) {
            impl->distributedObjectListenerService.addDistributedObject(name);
        };

        void HazelcastClient::triggerDistributedObjectRemoved(const std::string& name) {
            impl->distributedObjectListenerService.removeDistributedObject(name);
        };

        serialization::SerializationService& HazelcastClient::getSerializationService() {
            return impl->serializationService;
        };

        ClientConfig& HazelcastClient::getClientConfig() {
            return impl->clientConfig;
        };

        spi::ClientContext & HazelcastClient::getClientContext() {
            return impl->clientContext;
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

        spi::LifecycleService & HazelcastClient::getLifecycleService() {
            return impl->lifecycleService;
        };

        spi::ServerListenerService& HazelcastClient::getServerListenerService() {
            return impl->serverListenerService;
        };

        spi::DistributedObjectListenerService& HazelcastClient::getDistributedObjectListenerService() {
            return impl->distributedObjectListenerService;
        };

        connection::ConnectionManager & HazelcastClient::getConnectionManager() {
            return *(impl->connectionManager);
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

        ILock HazelcastClient::getILock(const std::string& instanceName) {
            return getDistributedObject< ILock >(instanceName);
        };

        IExecutorService HazelcastClient::getExecutorService(const std::string& instanceName) {
            return getDistributedObject< IExecutorService >(instanceName);
        }

        void HazelcastClient::addDistributedObjectListener(DistributedObjectListener *distributedObjectListener) {
            impl->distributedObjectListenerService.addDistributedObjectListener(distributedObjectListener);
        };

        bool HazelcastClient::removeDistributedObjectListener(DistributedObjectListener *distributedObjectListener) {
            impl->distributedObjectListenerService.removeDistributedObjectListener(distributedObjectListener);
        };

        TransactionContext HazelcastClient::newTransactionContext() {
            TransactionOptions defaultOptions;
            return newTransactionContext(defaultOptions);
        }

        TransactionContext HazelcastClient::newTransactionContext(const TransactionOptions& options) {
            return TransactionContext(impl->clusterService, impl->serializationService, *(impl->connectionManager), options);
        }

//        void shutdown() {
        //TODO shutdown the threads ????
//            partitionService.stop();
//            clusterService.stop();
//            connectionManager.shutdown();
//        }
//


    }
}