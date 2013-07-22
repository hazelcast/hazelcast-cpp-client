#include "hazelcast/client/IdGenerator.h"
#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/ILock.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient::HazelcastClientImpl {
        public:
            HazelcastClientImpl(ClientConfig& clientConfig, HazelcastClient& client)
            : clientConfig(clientConfig)
            , lifecycleService(client, this->clientConfig)
            , serializationService(0)
            , clusterService(partitionService, lifecycleService, connectionManager, serializationService, this->clientConfig)
            , connectionManager(clusterService, serializationService, this->clientConfig)
            , partitionService(clusterService, serializationService)
            , invocationService(clusterService, partitionService)
            , serverListenerService(invocationService)
            , cluster(clusterService)
            , clientContext(client) {
                LoadBalancer *loadBalancer = this->clientConfig.getLoadBalancer();
                loadBalancer->init(cluster);
            };


            ~HazelcastClientImpl(){
                //TODO shutdown the threads ????
                lifecycleService.setShutdown();
            }

            ClientConfig clientConfig;
            spi::LifecycleService lifecycleService;
            serialization::SerializationService serializationService;
            spi::ClusterService clusterService;
            connection::ConnectionManager connectionManager;
            spi::PartitionService partitionService;
            spi::InvocationService invocationService;
            spi::ServerListenerService serverListenerService;
            Cluster cluster;
            spi::ClientContext clientContext;

        };

        HazelcastClient::HazelcastClient(ClientConfig& config)
        :impl(new HazelcastClientImpl(config, *this)) {
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

        connection::ConnectionManager & HazelcastClient::getConnectionManager() {
            return impl->connectionManager;
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
            return getDistributedObject< ILock >(instanceName);;
        };



//        @Override TODO
//        public String addDistributedObjectListener(DistributedObjectListener distributedObjectListener) {
//        return proxyManager.addDistributedObjectListener(distributedObjectListener);
//    }
//
//        public boolean removeDistributedObjectListener(String registrationId) {
//        return proxyManager.removeDistributedObjectListener(registrationId);
//    }

//        @Override
//        public IExecutorService getExecutorService(String name) {
//        return getDistributedObject(DistributedExecutorService.SERVICE_NAME, name);
//    }
//
//        @Override
//        public <T> T executeTransaction(TransactionalTask<T> task) throws TransactionException {
//        return executeTransaction(TransactionOptions.getDefault(), task);
//    }
//
//        @Override
//        public <T> T executeTransaction(TransactionOptions options, TransactionalTask<T> task) throws TransactionException {
//        final TransactionContext context = newTransactionContext(options);
//        context.beginTransaction();
//        try {
//            final T value = task.execute(context);
//            context.commitTransaction();
//            return value;
//        } catch (Throwable e) {
//            context.rollbackTransaction();
//            if (e instanceof TransactionException) {
//                throw (TransactionException) e;
//            }
//            if (e.getCause() instanceof TransactionException) {
//                throw (TransactionException) e.getCause();
//            }
//            if (e instanceof RuntimeException) {
//                throw (RuntimeException) e;
//            }
//            throw new TransactionException(e);
//        }
//    }
//
//        @Override
//        public TransactionContext newTransactionContext() {
//        return newTransactionContext(TransactionOptions.getDefault());
//    }
//
//        @Override
//        public TransactionContext newTransactionContext(TransactionOptions options) {
//        return new TransactionContextProxy(this, options);
//    }
//
//        void shutdown() {
//            CLIENTS.remove(id);
//            executionService.shutdown();
//            partitionService.stop();
//            clusterService.stop();
//            connectionManager.shutdown();
//        }
//
//        public static Collection<HazelcastInstance> getAllHazelcastClients() {
//        return Collections.<HazelcastInstance>unmodifiableCollection(CLIENTS.values());
//    }
//
//        public static void shutdownAll() {
//        for (HazelcastClientProxy proxy : CLIENTS.values()) {
//            try {
//                proxy.client.getLifecycleService().shutdown();
//            } catch (Exception ignored) {
//            }
//            proxy.client = null;
//        }
//        CLIENTS.clear();
//    }


    }
}