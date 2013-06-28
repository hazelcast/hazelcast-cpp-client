#include "IdGenerator.h"
#include "ICountDownLatch.h"
#include "ISemaphore.h"
#include "ClientConfig.h"
#include "Cluster.h"
#include "HazelcastClient.h"
#include "hazelcast/client/impl/IdGeneratorSupport.h"
#include "ILock.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient::HazelcastClientImpl {
        public:
            HazelcastClientImpl(ClientConfig& clientConfig, HazelcastClient& client)
            : clientConfig(clientConfig)
            , lifecycleService(client, clientConfig)
            , serializationService(0)
            , clusterService(partitionService, lifecycleService , connectionManager, serializationService, clientConfig)
            , connectionManager(clusterService, serializationService, clientConfig)
            , partitionService(clusterService, serializationService)
            , invocationService(clusterService, partitionService)
            , serverListenerService(invocationService)
            , cluster(clusterService)
            , clientContext(client)   {
                LoadBalancer *loadBalancer = this->clientConfig.getLoadBalancer();
                loadBalancer->init(cluster);
            };

            void shutdown() {
                //TODO
            };

            impl::IdGeneratorSupport idGeneratorSupport;
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
            IdGenerator generator = getDistributedObject< IdGenerator >(instanceName);
            generator.setIdGeneratorSupport(&(impl->idGeneratorSupport));
            return generator;
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

        void HazelcastClient::shutdown() {
            impl->shutdown();
        };


    }
}