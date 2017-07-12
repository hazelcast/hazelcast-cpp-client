/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "hazelcast/client/HazelcastClient.h"

#include "hazelcast/client/IdGenerator.h"
#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/ILock.h"
#include "hazelcast/client/connection/ConnectionManager.h"

#ifndef HAZELCAST_VERSION
#define HAZELCAST_VERSION "NOT_FOUND"
#endif

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4355) //for strerror	
#endif

namespace hazelcast {
    namespace client {
        HazelcastClient::HazelcastClient(ClientConfig &config)
        : clientConfig(config)
        , clientProperties(config)
        , shutdownLatch(1)
        , clientContext(*this)
        , lifecycleService(clientContext, clientConfig, shutdownLatch)
        , serializationService(config.getSerializationConfig())
        , connectionManager(new connection::ConnectionManager(clientContext, clientConfig.isSmart()))
        , nearCacheManager(serializationService)
        , clusterService(clientContext)
        , partitionService(clientContext)
        , invocationService(clientContext)
        , serverListenerService(clientContext)
        , cluster(clusterService)
        , proxyManager(clientContext)
        , TOPIC_RB_PREFIX("_hz_rb_") {
            std::stringstream prefix;
            (prefix << "[HazelcastCppClient" << HAZELCAST_VERSION << "] [" << clientConfig.getGroupConfig().getName() << "]" );
            util::ILogger::getLogger().setPrefix(prefix.str());
            LoadBalancer *loadBalancer = clientConfig.getLoadBalancer();

            try {
                if (!lifecycleService.start()) {
                    throw exception::IllegalStateException("HazelcastClient","HazelcastClient could not be started!");
                }
            } catch (exception::IException &e) {
                lifecycleService.shutdown();
                throw;
            }
            loadBalancer->init(cluster);
        }

        HazelcastClient::~HazelcastClient() {
            lifecycleService.shutdown();
            /**
             * We can not depend on the destruction order of the variables. lifecycleService may be destructed later
             * than the clientContext which is accessed by different service threads, hence we need to explicitly wait
             * for shutdown completion.
             */
            shutdownLatch.await();
        }


        ClientConfig &HazelcastClient::getClientConfig() {
            return clientConfig;
        }

        Cluster &HazelcastClient::getCluster() {
            return cluster;
        }

        void HazelcastClient::addLifecycleListener(LifecycleListener *lifecycleListener) {
            lifecycleService.addLifecycleListener(lifecycleListener);
        }

        bool HazelcastClient::removeLifecycleListener(LifecycleListener *lifecycleListener) {
            return lifecycleService.removeLifecycleListener(lifecycleListener);
        }

        void HazelcastClient::shutdown() {
            lifecycleService.shutdown();
        }

        IdGenerator HazelcastClient::getIdGenerator(const std::string &instanceName) {
            return getDistributedObject< IdGenerator >(instanceName);
        }

        IAtomicLong HazelcastClient::getIAtomicLong(const std::string &instanceName) {
            return getDistributedObject< IAtomicLong >(instanceName);
        }

        ICountDownLatch HazelcastClient::getICountDownLatch(const std::string &instanceName) {
            return getDistributedObject< ICountDownLatch >(instanceName);
        }

        ISemaphore HazelcastClient::getISemaphore(const std::string &instanceName) {
            return getDistributedObject< ISemaphore >(instanceName);
        }

        ILock HazelcastClient::getILock(const std::string &instanceName) {
            return getDistributedObject< ILock >(instanceName);
        }

        TransactionContext HazelcastClient::newTransactionContext() {
            TransactionOptions defaultOptions;
            return newTransactionContext(defaultOptions);
        }

        TransactionContext HazelcastClient::newTransactionContext(const TransactionOptions &options) {
            return TransactionContext(clientContext, options);
        }

        internal::nearcache::NearCacheManager &HazelcastClient::getNearCacheManager() {
            return nearCacheManager;
        }

        serialization::pimpl::SerializationService &HazelcastClient::getSerializationService() {
            return serializationService;
        }

        boost::shared_ptr<spi::ClientProxy> HazelcastClient::getDistributedObjectForService(
                const std::string &serviceName,
                const std::string &name,
                spi::ClientProxyFactory &factory) {
            return proxyManager.getOrCreateProxy(serviceName, name, factory);
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

