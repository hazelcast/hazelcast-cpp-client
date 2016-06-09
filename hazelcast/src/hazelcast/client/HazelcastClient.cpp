/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_VERSION
#define HAZELCAST_VERSION "NOT_FOUND"
#endif

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4355) //for strerror	
#endif

namespace hazelcast {
    namespace client {
        const std::string HazelcastClient::TOPIC_RB_PREFIX = "_hz_rb_";;

        HazelcastClient::HazelcastClient(ClientConfig &config)
        : clientConfig(config)
        , clientProperties(config)
        , clientContext(*this)
        , lifecycleService(clientContext, clientConfig)
        , serializationService(config.getSerializationConfig())
        , connectionManager(clientContext, clientConfig.isSmart())
        , clusterService(clientContext)
        , partitionService(clientContext)
        , invocationService(clientContext)
        , serverListenerService(clientContext)
        , cluster(clusterService) {
            std::stringstream prefix;
            (prefix << "[HazelcastCppClient" << HAZELCAST_VERSION << "] [" << clientConfig.getGroupConfig().getName() << "]" );
            util::ILogger::getLogger().setPrefix(prefix.str());
            LoadBalancer *loadBalancer = clientConfig.getLoadBalancer();

            if (!lifecycleService.start()) {
                lifecycleService.shutdown();
                throw exception::IllegalStateException("HazelcastClient","HazelcastClient could not be started!");
            }
            loadBalancer->init(cluster);
        }

        HazelcastClient::~HazelcastClient() {
            lifecycleService.shutdown();
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

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

