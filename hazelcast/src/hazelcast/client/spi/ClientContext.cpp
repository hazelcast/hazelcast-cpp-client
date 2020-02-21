/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 5/23/13.

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"
#include "hazelcast/client/impl/ClientLockReferenceIdGenerator.h"
#include "hazelcast/client/spi/ClientInvocationService.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            ClientContext::ClientContext() {}

            ClientContext::ClientContext(const client::HazelcastClient &hazelcastClient) : hazelcastClient(
                    hazelcastClient.clientImpl) {
            }

            ClientContext::ClientContext(
                    const std::shared_ptr<client::impl::HazelcastClientInstanceImpl> &hazelcastClient)
                    : hazelcastClient(hazelcastClient) {
            }

            serialization::pimpl::SerializationService &ClientContext::getSerializationService() {
                return getHazelcastClientImplementationChecked()->serializationService;
            }

            ClientClusterService &ClientContext::getClientClusterService() {
                return getHazelcastClientImplementationChecked()->clusterService;
            }

            ClientInvocationService &ClientContext::getInvocationService() {
                return *getHazelcastClientImplementationChecked()->invocationService;
            }

            ClientConfig &ClientContext::getClientConfig() {
                return getHazelcastClientImplementationChecked()->clientConfig;
            }

            ClientPartitionService &ClientContext::getPartitionService() {
                return *getHazelcastClientImplementationChecked()->partitionService;
            }

            LifecycleService &ClientContext::getLifecycleService() {
                return getHazelcastClientImplementationChecked()->lifecycleService;
            }

            ClientListenerService &ClientContext::getClientListenerService() {
                return *getHazelcastClientImplementationChecked()->listenerService;
            }

            connection::ClientConnectionManagerImpl &ClientContext::getConnectionManager() {
                return *getHazelcastClientImplementationChecked()->connectionManager;
            }

            internal::nearcache::NearCacheManager &ClientContext::getNearCacheManager() {
                return *getHazelcastClientImplementationChecked()->nearCacheManager;
            }

            ClientProperties &ClientContext::getClientProperties() {
                return getHazelcastClientImplementationChecked()->clientProperties;
            }

            Cluster &ClientContext::getCluster() {
                return getHazelcastClientImplementationChecked()->cluster;
            }

            std::shared_ptr<impl::sequence::CallIdSequence> &ClientContext::getCallIdSequence() const {
                return getHazelcastClientImplementationChecked()->callIdSequence;
            }

            const protocol::ClientExceptionFactory &ClientContext::getClientExceptionFactory() const {
                return getHazelcastClientImplementationChecked()->getExceptionFactory();
            }

            const std::string &ClientContext::getName() const {
                return getHazelcastClientImplementationChecked()->getName();
            }

            impl::ClientExecutionServiceImpl &ClientContext::getClientExecutionService() const {
                return *getHazelcastClientImplementationChecked()->executionService;
            }

            void ClientContext::onClusterConnect(const std::shared_ptr<connection::Connection> &ownerConnection) {
                getHazelcastClientImplementationChecked()->onClusterConnect(ownerConnection);
            }

            const std::shared_ptr<client::impl::ClientLockReferenceIdGenerator> &
            ClientContext::getLockReferenceIdGenerator() {
                return getHazelcastClientImplementationChecked()->getLockReferenceIdGenerator();
            }

            std::shared_ptr<client::impl::HazelcastClientInstanceImpl>
            ClientContext::getHazelcastClientImplementation() {
                return hazelcastClient.lock();
            }

            void ClientContext::setClientImplementation(
                    const std::shared_ptr<client::impl::HazelcastClientInstanceImpl> &clientImpl) {
                hazelcastClient = clientImpl;
            }

            spi::ProxyManager &ClientContext::getProxyManager() {
                return getHazelcastClientImplementationChecked()->getProxyManager();
            }

            util::ILogger &ClientContext::getLogger() {
                return *getHazelcastClientImplementationChecked()->logger;
            }

            client::impl::statistics::Statistics &ClientContext::getClientstatistics() {
                return *getHazelcastClientImplementationChecked()->statistics;
            }

            std::shared_ptr<client::impl::HazelcastClientInstanceImpl>
            ClientContext::getHazelcastClientImplementationChecked() const {
                auto client = hazelcastClient.lock();
                if (!client) {
                    throw exception::IllegalStateException("ClientContext::getHazelcastClientImplementationChecked",
                                                           "Client is already shutdown and destructed.");
                }
                return client;
            }

        }

    }
}
