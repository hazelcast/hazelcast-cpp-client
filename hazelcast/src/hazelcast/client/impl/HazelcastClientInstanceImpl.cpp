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
#include "hazelcast/client/crdt/pncounter/impl/PNCounterProxyFactory.h"
#include "hazelcast/client/proxy/ClientPNCounterProxy.h"
#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"
#include "hazelcast/client/impl/ClientLockReferenceIdGenerator.h"
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
#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/ILock.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/mixedtype/impl/HazelcastClientImpl.h"
#include "hazelcast/client/flakeidgen/impl/FlakeIdGeneratorProxyFactory.h"
#include "hazelcast/client/idgen/impl/IdGeneratorProxyFactory.h"
#include "hazelcast/client/proxy/ClientFlakeIdGeneratorProxy.h"
#include "hazelcast/client/proxy/ClientIdGeneratorProxy.h"

#ifndef HAZELCAST_VERSION
#define HAZELCAST_VERSION "NOT_FOUND"
#endif

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4355) //for strerror	
#endif

_INITIALIZE_EASYLOGGINGPP

namespace hazelcast {
    namespace client {
        namespace impl {
            util::Atomic<int32_t> HazelcastClientInstanceImpl::CLIENT_ID(0);

            HazelcastClientInstanceImpl::HazelcastClientInstanceImpl(ClientConfig &config)
                    : clientConfig(config), clientProperties(clientConfig), shutdownLatch(1), clientContext(*this),
                      serializationService(clientConfig.getSerializationConfig()), clusterService(clientContext),
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

                initLogger();

                initalizeNearCacheManager();

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

                proxyManager.init();

                lockReferenceIdGenerator.reset(new impl::ClientLockReferenceIdGenerator());

                statistics.reset(new statistics::Statistics(clientContext));

                lifecycleService.fireLifecycleEvent(LifecycleEvent::STARTING);

                try {
                    if (!lifecycleService.start()) {
                        throw exception::IllegalStateException("HazelcastClient",
                                                               "HazelcastClient could not be started!");
                    }
                } catch (exception::IException &) {
                    lifecycleService.shutdown();
                    shutdownLatch.await();
                    throw;
                }
                mixedTypeSupportAdaptor.reset(new mixedtype::impl::HazelcastClientImpl(*this));
            }

            void HazelcastClientInstanceImpl::initLogger() {
                try {
                    logger.reset(new util::ILogger(instanceName, clientConfig.getGroupConfig().getName(), HAZELCAST_VERSION,
                                                   clientConfig.getLoggerConfig()));
                } catch (std::invalid_argument &ia) {
                    throw exception::IllegalStateException("HazelcastClientInstanceImpl::initLogger", ia.what());
                }
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
            }

            IdGenerator HazelcastClientInstanceImpl::getIdGenerator(const std::string &name) {
                idgen::impl::IdGeneratorProxyFactory factory(&clientContext);
                boost::shared_ptr<spi::ClientProxy> proxy =
                        getDistributedObjectForService(proxy::ClientIdGeneratorProxy::SERVICE_NAME, name, factory);

                boost::shared_ptr<impl::IdGeneratorInterface> impl = boost::static_pointer_cast<proxy::ClientIdGeneratorProxy>(
                        proxy);
                
                return IdGenerator(impl);
            }

            IAtomicLong HazelcastClientInstanceImpl::getIAtomicLong(const std::string &instanceName) {
                return getDistributedObject<IAtomicLong>(instanceName);
            }

            FlakeIdGenerator HazelcastClientInstanceImpl::getFlakeIdGenerator(const std::string &name) {
                flakeidgen::impl::FlakeIdGeneratorProxyFactory factory(&clientContext);
                boost::shared_ptr<spi::ClientProxy> proxy =
                        getDistributedObjectForService(proxy::ClientFlakeIdGeneratorProxy::SERVICE_NAME, name, factory);

                boost::shared_ptr<impl::IdGeneratorInterface> impl = boost::static_pointer_cast<proxy::ClientFlakeIdGeneratorProxy>(
                        proxy);

                return FlakeIdGenerator(impl);
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
                return *nearCacheManager;
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
                                                                  clientConfig.getExecutorPoolSize(), *logger));
            }

            std::auto_ptr<connection::ClientConnectionManagerImpl>
            HazelcastClientInstanceImpl::initConnectionManagerService(
                    const std::vector<boost::shared_ptr<connection::AddressProvider> > &addressProviders) {
                config::ClientAwsConfig &awsConfig = clientConfig.getNetworkConfig().getAwsConfig();
                boost::shared_ptr<connection::AddressTranslator> addressTranslator;
                if (awsConfig.isEnabled()) {
                    try {
                        addressTranslator.reset(new aws::impl::AwsAddressTranslator(awsConfig, *logger));
                    } catch (exception::InvalidConfigurationException &e) {
                        logger->warning(std::string("Invalid aws configuration! ") + e.what());
                        throw;
                    }
                } else {
                    addressTranslator.reset(new spi::impl::DefaultAddressTranslator());
                }
                return std::auto_ptr<connection::ClientConnectionManagerImpl>(
                        new connection::ClientConnectionManagerImpl(
                                clientContext, addressTranslator, addressProviders));

            }

            void HazelcastClientInstanceImpl::onClusterConnect(
                    const boost::shared_ptr<connection::Connection> &ownerConnection) {
                partitionService->listenPartitionTable(ownerConnection);
                clusterService.listenMembershipEvents(ownerConnection);
            }

            std::vector<boost::shared_ptr<connection::AddressProvider> >
            HazelcastClientInstanceImpl::createAddressProviders() {
                config::ClientNetworkConfig &networkConfig = getClientConfig().getNetworkConfig();
                config::ClientAwsConfig &awsConfig = networkConfig.getAwsConfig();
                std::vector<boost::shared_ptr<connection::AddressProvider> > addressProviders;

                if (awsConfig.isEnabled()) {
                    int awsMemberPort = clientProperties.getAwsMemberPort().getInteger();
                    if (awsMemberPort < 0 || awsMemberPort > 65535) {
                        throw (exception::ExceptionBuilder<exception::InvalidConfigurationException>(
                                "HazelcastClientInstanceImpl::createAddressProviders") << "Configured aws member port "
                                                                                       << awsMemberPort
                                                                                       << " is not a valid port number. It should be between 0-65535 inclusive.").build();
                    }
                    addressProviders.push_back(boost::shared_ptr<connection::AddressProvider>(
                            new spi::impl::AwsAddressProvider(awsConfig, awsMemberPort, *logger)));
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

            const boost::shared_ptr<ClientLockReferenceIdGenerator> &
            HazelcastClientInstanceImpl::getLockReferenceIdGenerator() const {
                return lockReferenceIdGenerator;
            }

            boost::shared_ptr<crdt::pncounter::PNCounter>
            HazelcastClientInstanceImpl::getPNCounter(const std::string &name) {
                crdt::pncounter::impl::PNCounterProxyFactory factory(&clientContext);
                boost::shared_ptr<spi::ClientProxy> proxy =
                        getDistributedObjectForService(proxy::ClientPNCounterProxy::SERVICE_NAME, name, factory);

                return boost::static_pointer_cast<proxy::ClientPNCounterProxy>(proxy);
            }

            spi::ProxyManager &HazelcastClientInstanceImpl::getProxyManager() {
                return proxyManager;
            }

            void HazelcastClientInstanceImpl::initalizeNearCacheManager() {
                nearCacheManager.reset(new internal::nearcache::NearCacheManager(serializationService, *logger));
            }

            boost::shared_ptr<IExecutorService>
            HazelcastClientInstanceImpl::getExecutorService(const std::string &name) {
/*
                crdt::pncounter::impl::PNCounterProxyFactory factory(&clientContext);
                boost::shared_ptr<spi::ClientProxy> proxy =
                        getDistributedObjectForService(proxy::ClientPNCounterProxy::SERVICE_NAME, name, factory);

                return boost::static_pointer_cast<proxy::ClientPNCounterProxy>(proxy);
*/
                return boost::shared_ptr<IExecutorService>();
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

