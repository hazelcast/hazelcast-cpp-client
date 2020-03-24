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

#include <vector>
#include <easylogging++.h>

#include "hazelcast/client/impl/BuildInfo.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IdGenerator.h"
#include "hazelcast/client/IAtomicLong.h"
#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/ILock.h"
#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/TransactionContext.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/LifecycleListener.h"
#include <hazelcast/client/executor/impl/ExecutorServiceProxyFactory.h>
#include <hazelcast/client/cluster/impl/ClusterDataSerializerHook.h>
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
#include "hazelcast/client/LoadBalancer.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/mixedtype/impl/HazelcastClientImpl.h"
#include "hazelcast/client/flakeidgen/impl/FlakeIdGeneratorProxyFactory.h"
#include "hazelcast/client/idgen/impl/IdGeneratorProxyFactory.h"
#include "hazelcast/client/proxy/ClientFlakeIdGeneratorProxy.h"
#include "hazelcast/client/proxy/ClientIdGeneratorProxy.h"
#include "hazelcast/client/proxy/ClientAtomicLongProxy.h"
#include "hazelcast/client/atomiclong/impl/AtomicLongProxyFactory.h"

#ifndef HAZELCAST_VERSION
#define HAZELCAST_VERSION "NOT_FOUND"
#endif

INITIALIZE_EASYLOGGINGPP

namespace hazelcast {
    namespace client {
        HazelcastClient::HazelcastClient() : clientImpl(new impl::HazelcastClientInstanceImpl(ClientConfig())) {
            clientImpl->start();
        }

        HazelcastClient::HazelcastClient(const ClientConfig &config) : clientImpl(
                new impl::HazelcastClientInstanceImpl(config)) {
            clientImpl->start();
        }

        const std::string &HazelcastClient::getName() const {
            return clientImpl->getName();
        }

        IdGenerator HazelcastClient::getIdGenerator(const std::string &name) {
            return clientImpl->getIdGenerator(name);
        }

        FlakeIdGenerator HazelcastClient::getFlakeIdGenerator(const std::string &name) {
            return clientImpl->getFlakeIdGenerator(name);
        }

        IAtomicLong HazelcastClient::getIAtomicLong(const std::string &name) {
            return clientImpl->getIAtomicLong(name);
        }

        std::shared_ptr<crdt::pncounter::PNCounter> HazelcastClient::getPNCounter(const std::string &name) {
            return clientImpl->getPNCounter(name);
        }

        ICountDownLatch HazelcastClient::getICountDownLatch(const std::string &name) {
            return clientImpl->getICountDownLatch(name);
        }

        ILock HazelcastClient::getILock(const std::string &name) {
            return clientImpl->getILock(name);
        }

        ISemaphore HazelcastClient::getISemaphore(const std::string &name) {
            return clientImpl->getISemaphore(name);
        }

        ClientConfig &HazelcastClient::getClientConfig() {
            return clientImpl->getClientConfig();
        }

        TransactionContext HazelcastClient::newTransactionContext() {
            return clientImpl->newTransactionContext();
        }

        TransactionContext HazelcastClient::newTransactionContext(const TransactionOptions &options) {
            return clientImpl->newTransactionContext(options);
        }

        Cluster &HazelcastClient::getCluster() {
            return clientImpl->getCluster();
        }

        void HazelcastClient::addLifecycleListener(LifecycleListener *lifecycleListener) {
            clientImpl->addLifecycleListener(lifecycleListener);
        }

        bool HazelcastClient::removeLifecycleListener(LifecycleListener *lifecycleListener) {
            return clientImpl->removeLifecycleListener(lifecycleListener);
        }

        void HazelcastClient::shutdown() {
            clientImpl->shutdown();
        }

        mixedtype::HazelcastClient &HazelcastClient::toMixedType() const {
            return clientImpl->toMixedType();
        }

        spi::LifecycleService &HazelcastClient::getLifecycleService() {
            return clientImpl->getLifecycleService();
        }

        std::shared_ptr<IExecutorService> HazelcastClient::getExecutorService(const std::string &name) {
            return clientImpl->getExecutorService(name);
        }

        Client HazelcastClient::getLocalEndpoint() const {
            return clientImpl->getLocalEndpoint();
        }

        HazelcastClient::~HazelcastClient() {
            clientImpl->shutdown();
        }

        namespace impl {
            std::atomic<int32_t> HazelcastClientInstanceImpl::CLIENT_ID(0);

            HazelcastClientInstanceImpl::HazelcastClientInstanceImpl(const ClientConfig &config)
                    : clientConfig(config), clientProperties(const_cast<ClientConfig &>(config).getProperties()),
                      clientContext(*this),
                      serializationService(clientConfig.getSerializationConfig()), clusterService(clientContext),
                      transactionManager(clientContext, *clientConfig.getLoadBalancer()), cluster(clusterService),
                      lifecycleService(clientContext, clientConfig.getLifecycleListeners(),
                                       clientConfig.getLoadBalancer(), cluster), proxyManager(clientContext),
                      id(++CLIENT_ID), TOPIC_RB_PREFIX("_hz_rb_") {
                const std::shared_ptr<std::string> &name = clientConfig.getInstanceName();
                if (name.get() != NULL) {
                    instanceName = *name;
                } else {
                    std::ostringstream out;
                    out << "hz.client_" << id;
                    instanceName = out.str();
                }

                logger.reset(new util::ILogger(instanceName, clientConfig.getGroupConfig().getName(), HAZELCAST_VERSION,
                                               clientConfig.getLoggerConfig()));

                initalizeNearCacheManager();

                executionService = initExecutionService();

                int32_t maxAllowedConcurrentInvocations = clientProperties.getInteger(
                        clientProperties.getMaxConcurrentInvocations());
                int64_t backofftimeoutMs = clientProperties.getLong(
                        clientProperties.getBackpressureBackoffTimeoutMillis());
                bool isBackPressureEnabled = maxAllowedConcurrentInvocations != INT32_MAX;
                callIdSequence = spi::impl::sequence::CallIdFactory::newCallIdSequence(isBackPressureEnabled,
                                                                                       maxAllowedConcurrentInvocations,
                                                                                       backofftimeoutMs);

                std::vector<std::shared_ptr<connection::AddressProvider>> addressProviders = createAddressProviders();

                connectionManager = initConnectionManagerService(addressProviders);

                partitionService.reset(new spi::impl::ClientPartitionServiceImpl(clientContext, *executionService));

                invocationService = initInvocationService();
                listenerService = initListenerService();

                proxyManager.init();

                lockReferenceIdGenerator.reset(new impl::ClientLockReferenceIdGenerator());

                statistics.reset(new statistics::Statistics(clientContext));
            }

            HazelcastClientInstanceImpl::~HazelcastClientInstanceImpl() {
            }

            void HazelcastClientInstanceImpl::start() {
                startLogger();

                lifecycleService.fireLifecycleEvent(LifecycleEvent::STARTING);

                try {
                    if (!lifecycleService.start()) {
                        try {
                            lifecycleService.shutdown();
                        } catch (exception::IException &e) {
                            logger->info("Lifecycle service start failed. Exception during shutdown: ", e.what());
                            // ignore
                        }
                        throw exception::IllegalStateException("HazelcastClient",
                                                               "HazelcastClient could not be started!");
                    }
                } catch (exception::IException &) {
                    try {
                        lifecycleService.shutdown();
                    } catch (exception::IException &e) {
                        logger->info("Exception during shutdown: ", e.what());
                        // ignore
                    }
                    throw;
                }

                mixedTypeSupportAdaptor = std::make_unique<mixedtype::impl::HazelcastClientImpl>(*this);
            }

            void HazelcastClientInstanceImpl::startLogger() {
                try {
                    if (!logger->start()) {
                        throw (exception::ExceptionBuilder<exception::IllegalStateException>(
                                "HazelcastClientInstanceImpl::initLogger")
                                << "Could not start logger for instance " << instanceName).build();
                    }
                } catch (std::invalid_argument &ia) {
                    throw exception::IllegalStateException("HazelcastClientInstanceImpl::initLogger", ia.what());
                }
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
                std::shared_ptr<spi::ClientProxy> proxy =
                        getDistributedObjectForService(proxy::ClientIdGeneratorProxy::SERVICE_NAME, name, factory);

                std::shared_ptr<impl::IdGeneratorInterface> impl = std::static_pointer_cast<proxy::ClientIdGeneratorProxy>(
                        proxy);

                return IdGenerator(impl);
            }

            IAtomicLong HazelcastClientInstanceImpl::getIAtomicLong(const std::string &name) {
                atomiclong::impl::AtomicLongProxyFactory factory(&clientContext);
                std::shared_ptr<spi::ClientProxy> proxy =
                        getDistributedObjectForService(proxy::ClientAtomicLongProxy::SERVICE_NAME, name, factory);

                std::shared_ptr<proxy::ClientAtomicLongProxy> impl = std::static_pointer_cast<proxy::ClientAtomicLongProxy>(
                        proxy);

                return IAtomicLong(impl);
            }

            FlakeIdGenerator HazelcastClientInstanceImpl::getFlakeIdGenerator(const std::string &name) {
                flakeidgen::impl::FlakeIdGeneratorProxyFactory factory(&clientContext);
                std::shared_ptr<spi::ClientProxy> proxy =
                        getDistributedObjectForService(proxy::ClientFlakeIdGeneratorProxy::SERVICE_NAME, name, factory);

                std::shared_ptr<impl::IdGeneratorInterface> impl = std::static_pointer_cast<proxy::ClientFlakeIdGeneratorProxy>(
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

            std::shared_ptr<spi::ClientProxy> HazelcastClientInstanceImpl::getDistributedObjectForService(
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

            std::shared_ptr<spi::ClientListenerService> HazelcastClientInstanceImpl::initListenerService() {
                int eventQueueCapacity = clientProperties.getInteger(clientProperties.getEventQueueCapacity());
                int eventThreadCount = clientProperties.getInteger(clientProperties.getEventThreadCount());
                config::ClientNetworkConfig &networkConfig = clientConfig.getNetworkConfig();
                if (networkConfig.isSmartRouting()) {
                    return std::shared_ptr<spi::ClientListenerService>(
                            new spi::impl::listener::SmartClientListenerService(clientContext, eventThreadCount,
                                                                                eventQueueCapacity));
                } else {
                    return std::shared_ptr<spi::ClientListenerService>(
                            new spi::impl::listener::NonSmartClientListenerService(clientContext, eventThreadCount,
                                                                                   eventQueueCapacity));
                }
            }

            std::unique_ptr<spi::ClientInvocationService> HazelcastClientInstanceImpl::initInvocationService() {
                if (clientConfig.getNetworkConfig().isSmartRouting()) {
                    return std::unique_ptr<spi::ClientInvocationService>(
                            new spi::impl::SmartClientInvocationService(clientContext));
                } else {
                    return std::unique_ptr<spi::ClientInvocationService>(
                            new spi::impl::NonSmartClientInvocationService(clientContext));
                }
            }

            std::shared_ptr<spi::impl::ClientExecutionServiceImpl>
            HazelcastClientInstanceImpl::initExecutionService() {
                return std::shared_ptr<spi::impl::ClientExecutionServiceImpl>(
                        new spi::impl::ClientExecutionServiceImpl(instanceName, clientProperties,
                                                                  clientConfig.getExecutorPoolSize(), *logger));
            }

            std::unique_ptr<connection::ClientConnectionManagerImpl>
            HazelcastClientInstanceImpl::initConnectionManagerService(
                    const std::vector<std::shared_ptr<connection::AddressProvider>> &addressProviders) {
                config::ClientAwsConfig &awsConfig = clientConfig.getNetworkConfig().getAwsConfig();
                std::shared_ptr<connection::AddressTranslator> addressTranslator;
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
                return std::unique_ptr<connection::ClientConnectionManagerImpl>(
                        new connection::ClientConnectionManagerImpl(
                                clientContext, addressTranslator, addressProviders));

            }

            void HazelcastClientInstanceImpl::onClusterConnect(
                    const std::shared_ptr<connection::Connection> &ownerConnection) {
                partitionService->listenPartitionTable(ownerConnection);
                clusterService.listenMembershipEvents(ownerConnection);
            }

            std::vector<std::shared_ptr<connection::AddressProvider>>
            HazelcastClientInstanceImpl::createAddressProviders() {
                config::ClientNetworkConfig &networkConfig = getClientConfig().getNetworkConfig();
                config::ClientAwsConfig &awsConfig = networkConfig.getAwsConfig();
                std::vector<std::shared_ptr<connection::AddressProvider>> addressProviders;

                if (awsConfig.isEnabled()) {
                    int awsMemberPort = clientProperties.getInteger(clientProperties.getAwsMemberPort());
                    if (awsMemberPort < 0 || awsMemberPort > 65535) {
                        throw (exception::ExceptionBuilder<exception::InvalidConfigurationException>(
                                "HazelcastClientInstanceImpl::createAddressProviders") << "Configured aws member port "
                                                                                       << awsMemberPort
                                                                                       << " is not a valid port number. It should be between 0-65535 inclusive.").build();
                    }
                    addressProviders.push_back(std::shared_ptr<connection::AddressProvider>(
                            new spi::impl::AwsAddressProvider(awsConfig, awsMemberPort, *logger)));
                }

                addressProviders.push_back(std::shared_ptr<connection::AddressProvider>(
                        new spi::impl::DefaultAddressProvider(networkConfig, addressProviders.empty())));

                return addressProviders;
            }

            const std::string &HazelcastClientInstanceImpl::getName() const {
                return instanceName;
            }

            spi::LifecycleService &HazelcastClientInstanceImpl::getLifecycleService() {
                return lifecycleService;
            }

            const std::shared_ptr<ClientLockReferenceIdGenerator> &
            HazelcastClientInstanceImpl::getLockReferenceIdGenerator() const {
                return lockReferenceIdGenerator;
            }

            std::shared_ptr<crdt::pncounter::PNCounter>
            HazelcastClientInstanceImpl::getPNCounter(const std::string &name) {
                crdt::pncounter::impl::PNCounterProxyFactory factory(&clientContext);
                std::shared_ptr<spi::ClientProxy> proxy =
                        getDistributedObjectForService(proxy::ClientPNCounterProxy::SERVICE_NAME, name, factory);

                return std::static_pointer_cast<proxy::ClientPNCounterProxy>(proxy);
            }

            spi::ProxyManager &HazelcastClientInstanceImpl::getProxyManager() {
                return proxyManager;
            }

            void HazelcastClientInstanceImpl::initalizeNearCacheManager() {
                nearCacheManager.reset(new internal::nearcache::NearCacheManager(serializationService, *logger));
            }

            std::shared_ptr<IExecutorService>
            HazelcastClientInstanceImpl::getExecutorService(const std::string &name) {
                executor::impl::ExecutorServiceProxyFactory factory(&clientContext);
                std::shared_ptr<spi::ClientProxy> proxy =
                        getDistributedObjectForService(IExecutorService::SERVICE_NAME, name, factory);

                return std::static_pointer_cast<IExecutorService>(proxy);
            }

            Client HazelcastClientInstanceImpl::getLocalEndpoint() const {
                return clusterService.getLocalClient();
            }

            const std::shared_ptr<util::ILogger> &HazelcastClientInstanceImpl::getLogger() const {
                return logger;
            }

            BaseEventHandler::~BaseEventHandler() {
            }

            void BaseEventHandler::handle(const std::shared_ptr<protocol::ClientMessage> &event) {
                std::unique_ptr<protocol::ClientMessage> e(new protocol::ClientMessage(*event));
                handle(std::move(e));
            }

            BaseEventHandler::BaseEventHandler() : logger(NULL) {}

            void BaseEventHandler::setLogger(util::ILogger *logger) {
                BaseEventHandler::logger = logger;
            }

            util::ILogger *BaseEventHandler::getLogger() const {
                return logger;
            }

            int BuildInfo::calculateVersion(const std::string &version) {
                std::vector<std::string> versionTokens = util::StringUtil::tokenizeVersionString(version);
                if (!versionTokens.empty()) {
                    int calculatedVersion = MAJOR_VERSION_MULTIPLIER * util::IOUtil::to_value<int>(versionTokens[0])
                                            + MINOR_VERSION_MULTIPLIER *
                                              util::IOUtil::to_value<int>(versionTokens[1]);
                    if (versionTokens.size() > PATCH_TOKEN_INDEX) {
                        size_t snapshotStartPosition = versionTokens[PATCH_TOKEN_INDEX].find("-");
                        if (snapshotStartPosition == std::string::npos) {
                            calculatedVersion += util::IOUtil::to_value<int>(versionTokens[PATCH_TOKEN_INDEX]);
                        }

                        if (snapshotStartPosition > 0) {
                            calculatedVersion += util::IOUtil::to_value<int>(
                                    versionTokens[PATCH_TOKEN_INDEX].substr(0, snapshotStartPosition));
                        }
                    }
                    return calculatedVersion;
                }

                return UNKNOWN_HAZELCAST_VERSION;
            }
        }

        LoadBalancer::~LoadBalancer() {
        }

        const int Address::ID = cluster::impl::ADDRESS;

        const byte Address::IPV4 = 4;
        const byte Address::IPV6 = 6;

        Address::Address() : host("localhost"), type(IPV4), scopeId(0) {
        }

        Address::Address(const std::string &url, int port)
                : host(url), port(port), type(IPV4), scopeId(0) {
        }

        Address::Address(const std::string &hostname, int port, unsigned long scopeId) : host(hostname), port(port),
                                                                                         type(IPV6), scopeId(scopeId) {
        }

        bool Address::operator==(const Address &rhs) const {
            return rhs.port == port && rhs.type == type && 0 == rhs.host.compare(host);
        }

        bool Address::operator!=(const Address &rhs) const {
            return !(*this == rhs);
        }

        int Address::getPort() const {
            return port;
        }

        const std::string &Address::getHost() const {
            return host;
        }

        int Address::getFactoryId() const {
            return cluster::impl::F_ID;
        }

        int Address::getClassId() const {
            return ID;
        }

        void Address::writeData(serialization::ObjectDataOutput &out) const {
            out.writeInt(port);
            out.writeByte(type);
            int len = (int) host.size();
            out.writeInt(len);
            out.writeBytes((const byte *) host.c_str(), len);
        }

        void Address::readData(serialization::ObjectDataInput &in) {
            port = in.readInt();
            type = in.readByte();
            int len = in.readInt();
            if (len > 0) {
                std::vector<byte> bytes;
                in.readFully(bytes);
                host.clear();
                host.append(bytes.begin(), bytes.end());
            }
        }

        bool Address::operator<(const Address &rhs) const {
            if (host < rhs.host) {
                return true;
            }
            if (rhs.host < host) {
                return false;
            }
            if (port < rhs.port) {
                return true;
            }
            if (rhs.port < port) {
                return false;
            }
            return type < rhs.type;
        }

        bool Address::isIpV4() const {
            return type == IPV4;
        }

        unsigned long Address::getScopeId() const {
            return scopeId;
        }

        std::string Address::toString() const {
            std::ostringstream out;
            out << "Address[" << getHost() << ":" << getPort() << "]";
            return out.str();
        }

        std::ostream &operator<<(std::ostream &stream, const Address &address) {
            return stream << address.toString();
        }

        LifecycleListener::~LifecycleListener() {
        }

        const std::string IExecutorService::SERVICE_NAME = "hz:impl:executorService";

        IExecutorService::IExecutorService(const std::string &name, spi::ClientContext *context) : ProxyImpl(
                SERVICE_NAME, name, context), consecutiveSubmits(0), lastSubmitTime(0) {
        }

        std::vector<Member>
        IExecutorService::selectMembers(const cluster::memberselector::MemberSelector &memberSelector) {
            std::vector<Member> selected;
            std::vector<Member> members = getContext().getClientClusterService().getMemberList();
            for (const Member &member : members) {
                if (memberSelector.select(member)) {
                    selected.push_back(member);
                }
            }
            if (selected.empty()) {
                throw (exception::ExceptionBuilder<exception::RejectedExecutionException>(
                        "IExecutorService::selectMembers") << "No member selected with memberSelector["
                                                           << memberSelector << "]").build();
            }
            return selected;
        }

        std::shared_ptr<spi::impl::ClientInvocationFuture>
        IExecutorService::invokeOnTarget(std::unique_ptr<protocol::ClientMessage> &request, const Address &target) {
            try {
                std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), target);
                return clientInvocation->invoke();
            } catch (exception::IException &e) {
                util::ExceptionUtil::rethrow(e);
            }
            return std::shared_ptr<spi::impl::ClientInvocationFuture>();
        }

        std::shared_ptr<spi::impl::ClientInvocationFuture>
        IExecutorService::invokeOnPartitionOwner(std::unique_ptr<protocol::ClientMessage> &request, int partitionId) {
            try {
                std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), partitionId);
                return clientInvocation->invoke();
            } catch (exception::IException &e) {
                util::ExceptionUtil::rethrow(e);
            }
            return std::shared_ptr<spi::impl::ClientInvocationFuture>();
        }

        bool IExecutorService::isSyncComputation(bool preventSync) {
            int64_t now = util::currentTimeMillis();

            int64_t last = lastSubmitTime;
            lastSubmitTime = now;

            if (last + MIN_TIME_RESOLUTION_OF_CONSECUTIVE_SUBMITS < now) {
                consecutiveSubmits = 0;
                return false;
            }

            return !preventSync && (consecutiveSubmits++ % MAX_CONSECUTIVE_SUBMITS == 0);
        }

        Address IExecutorService::getMemberAddress(const Member &member) {
            std::shared_ptr<Member> m = getContext().getClientClusterService().getMember(member.getUuid());
            if (m.get() == NULL) {
                throw (exception::ExceptionBuilder<exception::HazelcastException>(
                        "IExecutorService::getMemberAddress(Member)") << member << " is not available!").build();
            }
            return m->getAddress();
        }

        int IExecutorService::randomPartitionId() {
            spi::ClientPartitionService &partitionService = getContext().getPartitionService();
            return rand() % partitionService.getPartitionCount();
        }

        void IExecutorService::shutdown() {
            std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ExecutorServiceShutdownCodec::encodeRequest(
                    getName());
            invoke(request);
        }

        bool IExecutorService::isShutdown() {
            std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ExecutorServiceIsShutdownCodec::encodeRequest(
                    getName());
            return invokeAndGetResult<bool, protocol::codec::ExecutorServiceIsShutdownCodec::ResponseParameters>(
                    request);
        }

        bool IExecutorService::isTerminated() {
            return isShutdown();
        }

        const std::string ClientProperties::PROP_HEARTBEAT_TIMEOUT = "hazelcast_client_heartbeat_timeout";
        const std::string ClientProperties::PROP_HEARTBEAT_TIMEOUT_DEFAULT = "60";
        const std::string ClientProperties::PROP_HEARTBEAT_INTERVAL = "hazelcast_client_heartbeat_interval";
        const std::string ClientProperties::PROP_HEARTBEAT_INTERVAL_DEFAULT = "10";
        const std::string ClientProperties::PROP_REQUEST_RETRY_COUNT = "hazelcast_client_request_retry_count";
        const std::string ClientProperties::PROP_REQUEST_RETRY_COUNT_DEFAULT = "20";
        const std::string ClientProperties::PROP_REQUEST_RETRY_WAIT_TIME = "hazelcast_client_request_retry_wait_time";
        const std::string ClientProperties::PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT = "1";

        const std::string ClientProperties::PROP_AWS_MEMBER_PORT = "hz-port";
        const std::string ClientProperties::PROP_AWS_MEMBER_PORT_DEFAULT = "5701";

        const std::string ClientProperties::CLEAN_RESOURCES_PERIOD_MILLIS = "hazelcast.client.internal.clean.resources.millis";
        const std::string ClientProperties::CLEAN_RESOURCES_PERIOD_MILLIS_DEFAULT = "100";

        const std::string ClientProperties::INVOCATION_RETRY_PAUSE_MILLIS = "hazelcast.client.invocation.retry.pause.millis";
        const std::string ClientProperties::INVOCATION_RETRY_PAUSE_MILLIS_DEFAULT = "1000";

        const std::string ClientProperties::INVOCATION_TIMEOUT_SECONDS = "hazelcast.client.invocation.timeout.seconds";
        const std::string ClientProperties::INVOCATION_TIMEOUT_SECONDS_DEFAULT = "120";

        const std::string ClientProperties::EVENT_THREAD_COUNT = "hazelcast.client.event.thread.count";
        const std::string ClientProperties::EVENT_THREAD_COUNT_DEFAULT = "5";

        const std::string ClientProperties::EVENT_QUEUE_CAPACITY = "hazelcast.client.event.queue.capacity";
        const std::string ClientProperties::EVENT_QUEUE_CAPACITY_DEFAULT = "1000000";

        const std::string ClientProperties::INTERNAL_EXECUTOR_POOL_SIZE = "hazelcast.client.internal.executor.pool.size";
        const std::string ClientProperties::INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT = "3";

        const std::string ClientProperties::SHUFFLE_MEMBER_LIST = "hazelcast.client.shuffle.member.list";
        const std::string ClientProperties::SHUFFLE_MEMBER_LIST_DEFAULT = "true";

        const std::string ClientProperties::MAX_CONCURRENT_INVOCATIONS = "hazelcast.client.max.concurrent.invocations";
        const std::string ClientProperties::MAX_CONCURRENT_INVOCATIONS_DEFAULT = util::IOUtil::to_string<int32_t>(
                INT32_MAX);

        const std::string ClientProperties::BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS = "hazelcast.client.invocation.backoff.timeout.millis";
        const std::string ClientProperties::BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS_DEFAULT = "-1";

        const std::string ClientProperties::STATISTICS_ENABLED = "hazelcast.client.statistics.enabled";
        const std::string ClientProperties::STATISTICS_ENABLED_DEFAULT = "false";

        const std::string ClientProperties::STATISTICS_PERIOD_SECONDS = "hazelcast.client.statistics.period.seconds";
        const std::string ClientProperties::STATISTICS_PERIOD_SECONDS_DEFAULT = "3";

        const std::string ClientProperties::IO_THREAD_COUNT = "hazelcast.client.io.thread.count";
        const std::string ClientProperties::IO_THREAD_COUNT_DEFAULT = "1";

        ClientProperty::ClientProperty(const std::string &name, const std::string &defaultValue)
                : name(name), defaultValue(defaultValue) {
        }

        const std::string &ClientProperty::getName() const {
            return name;
        }

        const std::string &ClientProperty::getDefaultValue() const {
            return defaultValue;
        }

        const char *ClientProperty::getSystemProperty() const {
            return ::getenv(name.c_str());
        }

        ClientProperties::ClientProperties(const std::map<std::string, std::string> &properties)
                : heartbeatTimeout(PROP_HEARTBEAT_TIMEOUT, PROP_HEARTBEAT_TIMEOUT_DEFAULT),
                  heartbeatInterval(PROP_HEARTBEAT_INTERVAL, PROP_HEARTBEAT_INTERVAL_DEFAULT),
                  retryCount(PROP_REQUEST_RETRY_COUNT, PROP_REQUEST_RETRY_COUNT_DEFAULT),
                  retryWaitTime(PROP_REQUEST_RETRY_WAIT_TIME, PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT),
                  awsMemberPort(PROP_AWS_MEMBER_PORT, PROP_AWS_MEMBER_PORT_DEFAULT),
                  cleanResourcesPeriod(CLEAN_RESOURCES_PERIOD_MILLIS,
                                       CLEAN_RESOURCES_PERIOD_MILLIS_DEFAULT),
                  invocationRetryPauseMillis(INVOCATION_RETRY_PAUSE_MILLIS,
                                             INVOCATION_RETRY_PAUSE_MILLIS_DEFAULT),
                  invocationTimeoutSeconds(INVOCATION_TIMEOUT_SECONDS,
                                           INVOCATION_TIMEOUT_SECONDS_DEFAULT),
                  eventThreadCount(EVENT_THREAD_COUNT, EVENT_THREAD_COUNT_DEFAULT),
                  eventQueueCapacity(EVENT_QUEUE_CAPACITY, EVENT_QUEUE_CAPACITY_DEFAULT),
                  internalExecutorPoolSize(INTERNAL_EXECUTOR_POOL_SIZE,
                                           INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT),
                  shuffleMemberList(SHUFFLE_MEMBER_LIST, SHUFFLE_MEMBER_LIST_DEFAULT),
                  maxConcurrentInvocations(MAX_CONCURRENT_INVOCATIONS,
                                           MAX_CONCURRENT_INVOCATIONS_DEFAULT),
                  backpressureBackoffTimeoutMillis(BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS,
                                                   BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS_DEFAULT),
                  statisticsEnabled(STATISTICS_ENABLED, STATISTICS_ENABLED_DEFAULT),
                  statisticsPeriodSeconds(STATISTICS_PERIOD_SECONDS, STATISTICS_PERIOD_SECONDS_DEFAULT),
                  ioThreadCount(IO_THREAD_COUNT, IO_THREAD_COUNT_DEFAULT),
                  propertiesMap(properties) {
        }

        const ClientProperty &ClientProperties::getHeartbeatTimeout() const {
            return heartbeatTimeout;
        }

        const ClientProperty &ClientProperties::getHeartbeatInterval() const {
            return heartbeatInterval;
        }

        const ClientProperty &ClientProperties::getAwsMemberPort() const {
            return awsMemberPort;
        }

        const ClientProperty &ClientProperties::getCleanResourcesPeriodMillis() const {
            return cleanResourcesPeriod;
        }

        const ClientProperty &ClientProperties::getInvocationRetryPauseMillis() const {
            return invocationRetryPauseMillis;
        }

        const ClientProperty &ClientProperties::getInvocationTimeoutSeconds() const {
            return invocationTimeoutSeconds;
        }

        const ClientProperty &ClientProperties::getEventThreadCount() const {
            return eventThreadCount;
        }

        const ClientProperty &ClientProperties::getEventQueueCapacity() const {
            return eventQueueCapacity;
        }

        const ClientProperty &ClientProperties::getInternalExecutorPoolSize() const {
            return internalExecutorPoolSize;
        }

        const ClientProperty &ClientProperties::getShuffleMemberList() const {
            return shuffleMemberList;
        }

        const ClientProperty &ClientProperties::getMaxConcurrentInvocations() const {
            return maxConcurrentInvocations;
        }

        const ClientProperty &ClientProperties::getBackpressureBackoffTimeoutMillis() const {
            return backpressureBackoffTimeoutMillis;
        }

        const ClientProperty &ClientProperties::getStatisticsEnabled() const {
            return statisticsEnabled;
        }

        const ClientProperty &ClientProperties::getStatisticsPeriodSeconds() const {
            return statisticsPeriodSeconds;
        }

        const ClientProperty &ClientProperties::getIOThreadCount() const {
            return ioThreadCount;
        }

        std::string ClientProperties::getString(const ClientProperty &property) const {
            std::map<std::string, std::string>::const_iterator valueIt = propertiesMap.find(property.getName());
            if (valueIt != propertiesMap.end()) {
                return valueIt->second;
            }

            const char *value = property.getSystemProperty();
            if (value != NULL) {
                return value;
            }

            return property.getDefaultValue();
        }

        bool ClientProperties::getBoolean(const ClientProperty &property) const {
            return util::IOUtil::to_value<bool>(getString(property));
        }

        int32_t ClientProperties::getInteger(const ClientProperty &property) const {
            return util::IOUtil::to_value<int32_t>(getString(property));
        }

        int64_t ClientProperties::getLong(const ClientProperty &property) const {
            return util::IOUtil::to_value<int64_t>(getString(property));
        }

        namespace exception {
            IException::IException(const std::string &exceptionName, const std::string &source,
                                   const std::string &message, const std::string &details,
                                   int32_t errorNo, int32_t causeCode, bool isRuntime, bool retryable) : src(source),
                                                                                                         msg(message),
                                                                                                         details(details),
                                                                                                         errorCode(
                                                                                                                 errorNo),
                                                                                                         causeErrorCode(
                                                                                                                 causeCode),
                                                                                                         runtimeException(
                                                                                                                 isRuntime),
                                                                                                         retryable(
                                                                                                                 retryable) {
                std::ostringstream out;
                out << exceptionName << " {" << message << ". Details:" << details << " Error code:" << errorNo
                    << ", Cause error code:" << causeCode << "} at " + source;
                report = out.str();
            }

            IException::IException(const std::string &exceptionName, const std::string &source,
                                   const std::string &message, int32_t errorNo,
                                   int32_t causeCode, bool isRuntime, bool retryable) : src(source), msg(message),
                                                                                        errorCode(errorNo),
                                                                                        causeErrorCode(causeCode),
                                                                                        runtimeException(isRuntime),
                                                                                        retryable(retryable) {
                std::ostringstream out;
                out << exceptionName << " {" << message << " Error code:" << errorNo << ", Cause error code:"
                    << causeCode << "} at " + source;
                report = out.str();
            }

            IException::IException(const std::string &exceptionName, const std::string &source,
                                   const std::string &message, int32_t errorNo, bool isRuntime, bool retryable) : src(
                    source), msg(message), errorCode(errorNo), runtimeException(isRuntime), retryable(retryable) {
                std::ostringstream out;
                out << exceptionName << " {" << message << " Error code:" << errorNo << "} at " + source;
                report = out.str();
            }

            IException::IException(const std::string &exceptionName, const std::string &source,
                                   const std::string &message, int32_t errorNo,
                                   const std::shared_ptr<IException> &cause, bool isRuntime, bool retryable) : src(
                    source), msg(message),
                                                                                                               errorCode(
                                                                                                                       errorNo),
                                                                                                               cause(cause),
                                                                                                               runtimeException(
                                                                                                                       isRuntime),
                                                                                                               retryable(
                                                                                                                       retryable) {
                std::ostringstream out;
                out << exceptionName << " {" << message << " Error code:" << errorNo << ", Caused by:" << *cause
                    << "} at " + source;
                report = out.str();

            }

            IException::~IException() throw() {
            }

            char const *IException::what() const throw() {
                return report.c_str();
            }

            const std::string &IException::getSource() const {
                return src;
            }

            const std::string &IException::getMessage() const {
                return msg;
            }

            void IException::raise() const {
                throw *this;
            }

            std::ostream &operator<<(std::ostream &os, const IException &exception) {
                os << exception.what();
                return os;
            }

            const std::shared_ptr<IException> &IException::getCause() const {
                return cause;
            }

            std::unique_ptr<IException> IException::clone() const {
                return std::unique_ptr<IException>(new IException(*this));
            }

            const std::string &IException::getDetails() const {
                return details;
            }

            int32_t IException::getErrorCode() const {
                return errorCode;
            }

            int32_t IException::getCauseErrorCode() const {
                return causeErrorCode;
            }

            bool IException::isRuntimeException() const {
                return runtimeException;
            }

            bool IException::isRetryable() const {
                return retryable;
            }
        }

        namespace executor {
            namespace impl {
                ExecutorServiceProxyFactory::ExecutorServiceProxyFactory(spi::ClientContext *clientContext)
                        : clientContext(
                        clientContext) {}

                std::shared_ptr<spi::ClientProxy> ExecutorServiceProxyFactory::create(const std::string &id) {
                    return std::shared_ptr<spi::ClientProxy>(
                            new IExecutorService(id, clientContext));
                }
            }
        }

        namespace exception {
            UndefinedErrorCodeException::UndefinedErrorCodeException(const std::string &source,
                                                                     const std::string &message,
                                                                     int32_t errorCode, int64_t correlationId,
                                                                     std::string details)
                    : IException("UndefinedErrorCodeException", source, message, protocol::UNDEFINED, true),
                      error(errorCode),
                      messageCallId(correlationId),
                      detailedErrorMessage(details) {
            }

            int32_t UndefinedErrorCodeException::getUndefinedErrorCode() const {
                return error;
            }

            int64_t UndefinedErrorCodeException::getMessageCallId() const {
                return messageCallId;
            }

            const std::string &UndefinedErrorCodeException::getDetailedErrorMessage() const {
                return detailedErrorMessage;
            }

            UndefinedErrorCodeException::~UndefinedErrorCodeException() throw() {
            }

            std::unique_ptr<IException> UndefinedErrorCodeException::clone() const {
                return std::unique_ptr<IException>(new UndefinedErrorCodeException(*this));
            }

            void UndefinedErrorCodeException::raise() const {
                throw *this;
            }

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message,
                                                                     const std::string &details, int32_t causeCode)
                    : IException("RetryableHazelcastException", source, message, details, protocol::RETRYABLE_HAZELCAST,
                                 causeCode, true, true), HazelcastException(source, message, details, causeCode) {
            }

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message)
                    : IException("RetryableHazelcastException", source, message, protocol::RETRYABLE_HAZELCAST, true,
                                 true),
                      HazelcastException(source, message) {
            }

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message,
                                                                     int32_t causeCode) : IException(
                    "RetryableHazelcastException", source, message, protocol::RETRYABLE_HAZELCAST, causeCode, true,
                    true),
                                                                                          HazelcastException(source,
                                                                                                             message,
                                                                                                             causeCode) {}

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message,
                                                                     const std::shared_ptr<IException> &cause)
                    : IException("RetryableHazelcastException", source, message, protocol::RETRYABLE_HAZELCAST, cause,
                                 true, true),
                      HazelcastException(source, message, cause) {}

            MemberLeftException::MemberLeftException(const std::string &source, const std::string &message,
                                                     const std::string &details, int32_t causeCode)
                    : IException("MemberLeftException", source, message, details, protocol::MEMBER_LEFT, causeCode,
                                 true),
                      ExecutionException(source, message, details, causeCode),
                      RetryableHazelcastException(source, message, details, causeCode) {
            }

            MemberLeftException::MemberLeftException(const std::string &source, const std::string &message,
                                                     int32_t causeCode) : IException("MemberLeftException", source,
                                                                                     message, protocol::MEMBER_LEFT,
                                                                                     causeCode, true, true),
                                                                          ExecutionException(source, message,
                                                                                             causeCode),
                                                                          RetryableHazelcastException(source, message,
                                                                                                      causeCode) {
            }

            MemberLeftException::MemberLeftException(const std::string &source, const std::string &message)
                    : IException("MemberLeftException", source, message, protocol::MEMBER_LEFT, true, true),
                      ExecutionException(source, message), RetryableHazelcastException(source, message) {
            }

            void MemberLeftException::raise() const {
                throw *this;
            }

            std::unique_ptr<IException> MemberLeftException::clone() const {
                return std::unique_ptr<IException>(new MemberLeftException(*this));
            }
        }
    }
}
