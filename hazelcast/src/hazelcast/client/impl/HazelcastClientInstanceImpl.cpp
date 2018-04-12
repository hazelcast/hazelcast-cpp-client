/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"
#include "hazelcast/client/spi/impl/SmartClientInvocationService.h"
#include "hazelcast/client/spi/impl/NonSmartClientInvocationService.h"
#include "hazelcast/client/spi/impl/listener/NonSmartClientListenerService.h"
#include "hazelcast/client/spi/impl/listener/SmartClientListenerService.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/spi/impl/sequence/CallIdFactory.h"
#include "hazelcast/client/spi/impl/AwsAddressProvider.h"
#include "hazelcast/client/spi/impl/DefaultAddressProvider.h"
#include "hazelcast/client/aws/impl/AwsAddressTranslator.h"
#include "hazelcast/client/spi/impl/DefaultAddressTranslator.h"
#include "hazelcast/client/IdGenerator.h"
#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/ILock.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/mixedtype/impl/HazelcastClientImpl.h"

#ifndef HAZELCAST_VERSION
#define HAZELCAST_VERSION "NOT_FOUND"
#endif

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4355) //for strerror	
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            util::Atomic<int32_t> HazelcastClientInstanceImpl::CLIENT_ID(0);

            HazelcastClientInstanceImpl::HazelcastClientInstanceImpl(ClientConfig &config)
                    : clientConfig(config), clientProperties(clientConfig), shutdownLatch(1), clientContext(*this),
                      serializationService(clientConfig.getSerializationConfig()),
                      nearCacheManager(serializationService), clusterService(clientContext),
                      transactionManager(clientContext, *clientConfig.getLoadBalancer()),
                      cluster(clusterService),
                      lifecycleService(clientContext, clientConfig.getLifecycleListeners(), shutdownLatch,
                                       clientConfig.getLoadBalancer(), cluster), proxyManager(clientContext),
                      id(++CLIENT_ID), TOPIC_RB_PREFIX("_hz_rb_") {
                const boost::shared_ptr<std::string> &name = config.getInstanceName();
                if (name.get() != NULL) {
                    instanceName = *name;
                } else {
                    std::ostringstream out;
                    out << "hz.client_" << id;
                    instanceName = out.str();
                }

                std::stringstream prefix;
                prefix << instanceName << "[" << clientConfig.getGroupConfig().getName() << "] [" << HAZELCAST_VERSION
                       << "]";
                util::ILogger::getLogger().setPrefix(prefix.str());

                executionService = initExecutionService();

                int32_t maxAllowedConcurrentInvocations = clientProperties.getMaxConcurrentInvocations().get<int32_t>();
                int64_t backofftimeoutMs = clientProperties.getBackpressureBackoffTimeoutMillis().get<int64_t>();
                bool isBackPressureEnabled = maxAllowedConcurrentInvocations != INT32_MAX;
                callIdSequence = spi::impl::sequence::CallIdFactory::newCallIdSequence(isBackPressureEnabled,
                                                                                       maxAllowedConcurrentInvocations,
                                                                                       backofftimeoutMs);

                std::vector<boost::shared_ptr<connection::AddressProvider> > addressProviders = createAddressProviders();

                connectionManager = initConnectionManagerService(addressProviders);

                partitionService.reset(new spi::impl::ClientPartitionServiceImpl(clientContext));

                invocationService = initInvocationService();
                listenerService = initListenerService();

                try {
                    if (!lifecycleService.start()) {
                        throw exception::IllegalStateException("HazelcastClient", "HazelcastClient could not be started!");
                    }
                } catch (exception::IException &) {
                    lifecycleService.shutdown();
                    throw;
                }
                mixedTypeSupportAdaptor.reset(new mixedtype::impl::HazelcastClientImpl(*this));
            }

            HazelcastClientInstanceImpl::~HazelcastClientInstanceImpl() {
                lifecycleService.shutdown();
                /**
                 * We can not depend on the destruction order of the variables. lifecycleService may be destructed later
                 * than the clientContext which is accessed by different service threads, hence we need to explicitly wait
                 * for shutdown completion.
                 */
                shutdownLatch.await();
            }


            ClientConfig &HazelcastClientInstanceImpl::getClientConfig() {
                return clientConfig;
            }

            Cluster &HazelcastClientInstanceImpl::getCluster() {
                return cluster;
            }

            void HazelcastClientInstanceImpl::addLifecycleListener(LifecycleListener *lifecycleListener) {
                lifecycleService.addLifecycleListener(lifecycleListener);
            }

            bool HazelcastClientInstanceImpl::removeLifecycleListener(LifecycleListener *lifecycleListener) {
                return lifecycleService.removeLifecycleListener(lifecycleListener);
            }

            void HazelcastClientInstanceImpl::shutdown() {
                lifecycleService.shutdown();

                serializationService.dispose();
            }

            IdGenerator HazelcastClientInstanceImpl::getIdGenerator(const std::string &instanceName) {
                return getDistributedObject<IdGenerator>(instanceName);
            }

            IAtomicLong HazelcastClientInstanceImpl::getIAtomicLong(const std::string &instanceName) {
                return getDistributedObject<IAtomicLong>(instanceName);
            }

            ICountDownLatch HazelcastClientInstanceImpl::getICountDownLatch(const std::string &instanceName) {
                return getDistributedObject<ICountDownLatch>(instanceName);
            }

            ISemaphore HazelcastClientInstanceImpl::getISemaphore(const std::string &instanceName) {
                return getDistributedObject<ISemaphore>(instanceName);
            }

            ILock HazelcastClientInstanceImpl::getILock(const std::string &instanceName) {
                return getDistributedObject<ILock>(instanceName);
            }

            TransactionContext HazelcastClientInstanceImpl::newTransactionContext() {
                TransactionOptions defaultOptions;
                return newTransactionContext(defaultOptions);
            }

            TransactionContext HazelcastClientInstanceImpl::newTransactionContext(const TransactionOptions &options) {
                return TransactionContext(transactionManager, options);
            }

            internal::nearcache::NearCacheManager &HazelcastClientInstanceImpl::getNearCacheManager() {
                return nearCacheManager;
            }

            serialization::pimpl::SerializationService &HazelcastClientInstanceImpl::getSerializationService() {
                return serializationService;
            }

            boost::shared_ptr<spi::ClientProxy> HazelcastClientInstanceImpl::getDistributedObjectForService(
                    const std::string &serviceName,
                    const std::string &name,
                    spi::ClientProxyFactory &factory) {
                return proxyManager.getOrCreateProxy(serviceName, name, factory);
            }

            mixedtype::HazelcastClient &HazelcastClientInstanceImpl::toMixedType() const {
                return *mixedTypeSupportAdaptor;
            }

            const protocol::ClientExceptionFactory &HazelcastClientInstanceImpl::getExceptionFactory() const {
                return exceptionFactory;
            }

            boost::shared_ptr<spi::ClientListenerService> HazelcastClientInstanceImpl::initListenerService() {
                int eventQueueCapacity = clientProperties.getEventQueueCapacity().getInteger();
                int eventThreadCount = clientProperties.getEventThreadCount().getInteger();
                config::ClientNetworkConfig &networkConfig = clientConfig.getNetworkConfig();
                if (networkConfig.isSmartRouting()) {
                    return boost::shared_ptr<spi::ClientListenerService>(
                            new spi::impl::listener::SmartClientListenerService(clientContext, eventThreadCount,
                                                                                eventQueueCapacity));
                } else {
                    return boost::shared_ptr<spi::ClientListenerService>(
                            new spi::impl::listener::NonSmartClientListenerService(clientContext, eventThreadCount,
                                                                                   eventQueueCapacity));
                }
            }

            std::auto_ptr<spi::ClientInvocationService> HazelcastClientInstanceImpl::initInvocationService() {
                if (clientConfig.getNetworkConfig().isSmartRouting()) {
                    return std::auto_ptr<spi::ClientInvocationService>(
                            new spi::impl::SmartClientInvocationService(clientContext));
                } else {
                    return std::auto_ptr<spi::ClientInvocationService>(
                            new spi::impl::NonSmartClientInvocationService(clientContext));
                }
            }

            std::auto_ptr<spi::impl::ClientExecutionServiceImpl> HazelcastClientInstanceImpl::initExecutionService() {
                return std::auto_ptr<spi::impl::ClientExecutionServiceImpl>(
                        new spi::impl::ClientExecutionServiceImpl(instanceName, clientProperties,
                                                                  clientConfig.getExecutorPoolSize()));
            }

            std::auto_ptr<connection::ClientConnectionManagerImpl> HazelcastClientInstanceImpl::initConnectionManagerService(
                    const std::vector<boost::shared_ptr<connection::AddressProvider> > &addressProviders) {
                config::ClientAwsConfig &awsConfig = clientConfig.getNetworkConfig().getAwsConfig();
                boost::shared_ptr<connection::AddressTranslator> addressTranslator;
                if (awsConfig.isEnabled()) {
                    try {
                        addressTranslator.reset(new aws::impl::AwsAddressTranslator(awsConfig));
                    } catch (exception::InvalidConfigurationException &e) {
                        util::ILogger::getLogger().warning(std::string("Invalid aws configuration! ") + e.what());
                        throw;
                    }
                } else {
                    addressTranslator.reset(new spi::impl::DefaultAddressTranslator());
                }
                return std::auto_ptr<connection::ClientConnectionManagerImpl>(new connection::ClientConnectionManagerImpl(
                        clientContext, addressTranslator, addressProviders));

            }

            void HazelcastClientInstanceImpl::onClusterConnect(const boost::shared_ptr<connection::Connection> &ownerConnection) {
                partitionService->listenPartitionTable(ownerConnection);
                clusterService.listenMembershipEvents(ownerConnection);
            }

            std::vector<boost::shared_ptr<connection::AddressProvider> > HazelcastClientInstanceImpl::createAddressProviders() {
                config::ClientNetworkConfig &networkConfig = getClientConfig().getNetworkConfig();
                config::ClientAwsConfig &awsConfig = networkConfig.getAwsConfig();
                std::vector<boost::shared_ptr<connection::AddressProvider> > addressProviders;

                if (awsConfig.isEnabled()) {
                    addressProviders.push_back(boost::shared_ptr<connection::AddressProvider>(
                            new spi::impl::AwsAddressProvider(awsConfig, util::ILogger::getLogger())));
                }

                addressProviders.push_back(boost::shared_ptr<connection::AddressProvider>(
                        new spi::impl::DefaultAddressProvider(networkConfig, addressProviders.empty())));

                return addressProviders;
            }

            const std::string &HazelcastClientInstanceImpl::getName() const {
                return instanceName;
            }

            spi::LifecycleService &HazelcastClientInstanceImpl::getLifecycleService() {
                return lifecycleService;
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

