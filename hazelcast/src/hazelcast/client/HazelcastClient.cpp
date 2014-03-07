#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IdGenerator.h"
#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/ILock.h"

namespace hazelcast {
    namespace client {

        HazelcastClient::HazelcastClient(ClientConfig &config)
        : clientConfig(config)
        , clientContext(*this)
        , lifecycleService(clientContext, clientConfig)
        , serializationService(0)
        , connectionManager(clientContext, clientConfig.isSmart())
        , clusterService(clientContext)
        , partitionService(clientContext)
        , invocationService(clientContext)
        , serverListenerService(clientContext)
        , cluster(clusterService) {
            LoadBalancer *loadBalancer = clientConfig.getLoadBalancer();
            loadBalancer->init(cluster);
            if (!lifecycleService.start()) {
                lifecycleService.shutdown();
                throw exception::IllegalStateException("HazelcastClient","HazelcastClient could not started!");
            }
        };

        HazelcastClient::~HazelcastClient() {
            lifecycleService.shutdown();
        };


        ClientConfig &HazelcastClient::getClientConfig() {
            return clientConfig;
        };

        Cluster &HazelcastClient::getCluster() {
            return cluster;
        }

        void HazelcastClient::addLifecycleListener(LifecycleListener *lifecycleListener) {
            lifecycleService.addLifecycleListener(lifecycleListener);
        };

        bool HazelcastClient::removeLifecycleListener(LifecycleListener *lifecycleListener) {
            return lifecycleService.removeLifecycleListener(lifecycleListener);
        };

        void HazelcastClient::shutdown() {
            lifecycleService.shutdown();
        }

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
            return TransactionContext(clientContext, options);
        }

    }
}