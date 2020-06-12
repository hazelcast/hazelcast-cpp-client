/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#include <functional>
#include <easylogging++.h>
#include <boost/format.hpp>

#include "hazelcast/client/impl/BuildInfo.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/TransactionContext.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/LifecycleListener.h"
#include <hazelcast/client/cluster/impl/ClusterDataSerializerHook.h>
#include <hazelcast/client/exception/ProtocolExceptions.h>
#include "hazelcast/client/proxy/PNCounterImpl.h"
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
#include "hazelcast/client/LoadBalancer.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/proxy/FlakeIdGeneratorImpl.h"

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

        spi::LifecycleService &HazelcastClient::getLifecycleService() {
            return clientImpl->getLifecycleService();
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
                      id(++CLIENT_ID) {
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
                        lifecycleService.shutdown();
                        BOOST_THROW_EXCEPTION(exception::IllegalStateException("HazelcastClient",
                                                                               "HazelcastClient could not be started!"));
                    }
                } catch (exception::IException &) {
                    lifecycleService.shutdown();
                    throw;
                }
            }

            void HazelcastClientInstanceImpl::startLogger() {
                try {
                    if (!logger->start()) {
                        throw (exception::ExceptionBuilder<exception::IllegalStateException>(
                                "HazelcastClientInstanceImpl::initLogger")
                                << "Could not start logger for instance " << instanceName).build();
                    }
                } catch (std::invalid_argument &ia) {
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalStateException("HazelcastClientInstanceImpl::initLogger", ia.what()));
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

            const protocol::ClientExceptionFactory &HazelcastClientInstanceImpl::getExceptionFactory() const {
                return exceptionFactory;
            }

            std::shared_ptr<spi::ClientListenerService> HazelcastClientInstanceImpl::initListenerService() {
                int eventThreadCount = clientProperties.getInteger(clientProperties.getEventThreadCount());
                config::ClientNetworkConfig &networkConfig = clientConfig.getNetworkConfig();
                if (networkConfig.isSmartRouting()) {
                    return std::shared_ptr<spi::ClientListenerService>(
                            new spi::impl::listener::SmartClientListenerService(clientContext, eventThreadCount));
                } else {
                    return std::shared_ptr<spi::ClientListenerService>(
                            new spi::impl::listener::NonSmartClientListenerService(clientContext, eventThreadCount));
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
                return std::make_shared<spi::impl::ClientExecutionServiceImpl>(instanceName, clientProperties,
                                                                               clientConfig.getExecutorPoolSize(),
                                                                               lifecycleService);
            }

            std::shared_ptr<connection::ClientConnectionManagerImpl>
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
                return std::make_shared<connection::ClientConnectionManagerImpl>(
                        clientContext, addressTranslator, addressProviders);

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

            spi::ProxyManager &HazelcastClientInstanceImpl::getProxyManager() {
                return proxyManager;
            }

            void HazelcastClientInstanceImpl::initalizeNearCacheManager() {
                nearCacheManager.reset(
                        new internal::nearcache::NearCacheManager(executionService, serializationService, *logger));
            }

            Client HazelcastClientInstanceImpl::getLocalEndpoint() const {
                return clusterService.getLocalClient();
            }

            template<>
            std::shared_ptr<IMap> HazelcastClientInstanceImpl::getDistributedObject(const std::string& name) {
                auto nearCacheConfig = clientConfig.getNearCacheConfig<serialization::pimpl::Data, serialization::pimpl::Data>(
                        name);
                if (nearCacheConfig) {
                    return proxyManager.getOrCreateProxy<map::NearCachedClientMapProxy<serialization::pimpl::Data, serialization::pimpl::Data>>(
                            IMap::SERVICE_NAME, name);
                } else {
                    return proxyManager.getOrCreateProxy<IMap>(IMap::SERVICE_NAME, name);
                }
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

            void BaseEventHandler::setLogger(util::ILogger *iLogger) {
                BaseEventHandler::logger = iLogger;
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

        namespace serialization {
            int32_t hz_serializer<Address>::getFactoryId() {
                return F_ID;
            }

            int32_t hz_serializer<Address>::getClassId() {
                return ADDRESS;
            }

            void hz_serializer<Address>::writeData(const Address &object, ObjectDataOutput &out) {
                out.write<int32_t>(object.port);
                out.write<byte>(object.type);
                out.write(object.host);
            }

            Address hz_serializer<Address>::readData(ObjectDataInput &in) {
                Address object;
                object.port = in.read<int32_t>();
                object.type = in.read<byte>();
                object.host = in.read<std::string>();
                return object;
            }
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

        std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
        IExecutorService::invokeOnTarget(std::unique_ptr<protocol::ClientMessage> &request, const Address &target) {
            try {
                std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), target);
                return std::make_pair(clientInvocation->invoke(), clientInvocation);
            } catch (exception::IException &e) {
                util::ExceptionUtil::rethrow(std::current_exception());
            }
            return std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>();
        }

        std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
        IExecutorService::invokeOnPartitionOwner(std::unique_ptr<protocol::ClientMessage> &request, int partitionId) {
            try {
                std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), partitionId);
                return std::make_pair(clientInvocation->invoke(), clientInvocation);
            } catch (exception::IException &e) {
                util::ExceptionUtil::rethrow(std::current_exception());
            }
            return std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>();
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
            auto m = getContext().getClientClusterService().getMember(member.getUuid());
            if (!m) {
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
            auto request = protocol::codec::ExecutorServiceShutdownCodec::encodeRequest(
                    getName());
            invoke(request);
        }

        boost::future<bool> IExecutorService::isShutdown() {
            auto request = protocol::codec::ExecutorServiceIsShutdownCodec::encodeRequest(
                    getName());
            return invokeAndGetFuture<bool, protocol::codec::ExecutorServiceIsShutdownCodec::ResponseParameters>(
                    request);
        }

        boost::future<bool> IExecutorService::isTerminated() {
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

        const std::string ClientProperties::RESPONSE_EXECUTOR_THREAD_COUNT = "hazelcast.client.response.executor.thread.count";
        const std::string ClientProperties::RESPONSE_EXECUTOR_THREAD_COUNT_DEFAULT = "0";

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

        ClientProperties::ClientProperties(const std::unordered_map<std::string, std::string> &properties)
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
                  responseExecutorThreadCount(RESPONSE_EXECUTOR_THREAD_COUNT, RESPONSE_EXECUTOR_THREAD_COUNT_DEFAULT),
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
            std::unordered_map<std::string, std::string>::const_iterator valueIt = propertiesMap.find(property.getName());
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

        const ClientProperty &ClientProperties::getResponseExecutorThreadCount() const {
            return responseExecutorThreadCount;
        }

        namespace exception {
            IException::IException(const std::string &exceptionName, const std::string &source,
                                   const std::string &message, const std::string &details, int32_t errorNo,
                                   bool isRuntime, bool retryable)
                    : src(source), msg(message), details(details), errorCode(errorNo), runtimeException(isRuntime),
                      retryable(retryable), report((boost::format(
                            "%1% {%2%. Error code:%3%, Details:%4%.} at %5%.") % exceptionName % message % errorNo %
                                                    details % source).str()) {
            }

            IException::~IException() noexcept {
            }

            char const *IException::what() const noexcept {
                return report.c_str();
            }

            const std::string &IException::getSource() const {
                return src;
            }

            const std::string &IException::getMessage() const {
                return msg;
            }

            std::ostream &operator<<(std::ostream &os, const IException &exception) {
                os << exception.what();
                return os;
            }

            const std::string &IException::getDetails() const {
                return details;
            }

            int32_t IException::getErrorCode() const {
                return errorCode;
            }

            bool IException::isRuntimeException() const {
                return runtimeException;
            }

            bool IException::isRetryable() const {
                return retryable;
            }

            IException::IException() {}

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message,
                                                                     const std::string &details)
                    : RetryableHazelcastException(
                    "RetryableHazelcastException", protocol::RETRYABLE_HAZELCAST, source, message, details, true,
                    true) {}

            RetryableHazelcastException::RetryableHazelcastException(const std::string &errorName, int32_t errorCode,
                                                                     const std::string &source,
                                                                     const std::string &message,
                                                                     const std::string &details, bool runtime,
                                                                     bool retryable) : HazelcastException(errorName,
                                                                                                          errorCode,
                                                                                                          source,
                                                                                                          message,
                                                                                                          details,
                                                                                                          runtime,
                                                                                                          retryable) {}

            MemberLeftException::MemberLeftException(const std::string &source, const std::string &message,
                                                     const std::string &details)
                    : ExecutionException("MemberLeftException", protocol::MEMBER_LEFT, source, message, details, false,
                                         true) {}

            ConsistencyLostException::ConsistencyLostException(const std::string &source, const std::string &message,
                                                               const std::string &details)
                    : HazelcastException("ConsistencyLostException", protocol::CONSISTENCY_LOST, source, message,
                                         details, true,
                                         false) {}
        }
    }
}

namespace std {
    std::size_t hash<hazelcast::client::Address>::operator()(const hazelcast::client::Address &address) const noexcept {
        return std::hash<std::string>()(address.getHost()) + std::hash<int>()(address.getPort()) +
               std::hash<unsigned long>()(address.type);
    }
}
