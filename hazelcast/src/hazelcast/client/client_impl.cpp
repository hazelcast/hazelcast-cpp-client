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
#include <boost/format.hpp>

#include "hazelcast/util/Util.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/TransactionContext.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/LifecycleListener.h"
#include <hazelcast/client/cluster/impl/ClusterDataSerializerHook.h>
#include <hazelcast/client/exception/ProtocolExceptions.h>
#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"
#include "hazelcast/client/impl/ClientLockReferenceIdGenerator.h"
#include "hazelcast/client/spi/impl/ClientInvocationServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/spi/impl/sequence/CallIdFactory.h"
#include "hazelcast/client/spi/impl/AwsAddressProvider.h"
#include "hazelcast/client/spi/impl/DefaultAddressProvider.h"
#include "hazelcast/client/aws/impl/AwsAddressTranslator.h"
#include "hazelcast/client/spi/impl/DefaultAddressTranslator.h"
#include "hazelcast/client/spi/impl/listener/listener_service_impl.h"
#include "hazelcast/client/LoadBalancer.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/proxy/FlakeIdGeneratorImpl.h"
#include "hazelcast/logger.h"

#ifndef HAZELCAST_VERSION
#define HAZELCAST_VERSION "NOT_FOUND"
#endif

namespace hazelcast {
    namespace client {
        HazelcastClient::HazelcastClient() : clientImpl_(new impl::HazelcastClientInstanceImpl(ClientConfig())) {
            clientImpl_->start();
        }

        HazelcastClient::HazelcastClient(const ClientConfig &config) : clientImpl_(
                new impl::HazelcastClientInstanceImpl(config)) {
            clientImpl_->start();
        }

        const std::string &HazelcastClient::getName() const {
            return clientImpl_->getName();
        }

        ClientConfig &HazelcastClient::getClientConfig() {
            return clientImpl_->getClientConfig();
        }

        TransactionContext HazelcastClient::newTransactionContext() {
            return clientImpl_->newTransactionContext();
        }

        TransactionContext HazelcastClient::newTransactionContext(const TransactionOptions &options) {
            return clientImpl_->newTransactionContext(options);
        }

        Cluster &HazelcastClient::getCluster() {
            return clientImpl_->getCluster();
        }

        boost::uuids::uuid HazelcastClient::addLifecycleListener(LifecycleListener &&lifecycleListener) {
            return clientImpl_->addLifecycleListener(std::move(lifecycleListener));
        }

        bool HazelcastClient::removeLifecycleListener(const boost::uuids::uuid &registrationId) {
            return clientImpl_->removeLifecycleListener(registrationId);
        }

        void HazelcastClient::shutdown() {
            clientImpl_->shutdown();
        }

        spi::LifecycleService &HazelcastClient::getLifecycleService() {
            return clientImpl_->getLifecycleService();
        }

        Client HazelcastClient::getLocalEndpoint() const {
            return clientImpl_->getLocalEndpoint();
        }

        HazelcastClient::~HazelcastClient() {
            clientImpl_->shutdown();
        }

        cp::cp_subsystem &HazelcastClient::get_cp_subsystem() {
            return clientImpl_->get_cp_subsystem();
        }

        namespace impl {
            std::atomic<int32_t> HazelcastClientInstanceImpl::CLIENT_ID(0);

            HazelcastClientInstanceImpl::HazelcastClientInstanceImpl(const ClientConfig &config)
                    : clientConfig_(config), clientProperties_(config.getProperties()),
                      clientContext_(*this),
                      serializationService_(clientConfig_.getSerializationConfig()), clusterService_(clientContext_),
                      transactionManager_(clientContext_), cluster_(clusterService_),
                      lifecycleService_(clientContext_, clientConfig_.getLifecycleListeners(),
                                       clientConfig_.getLoadBalancer(), cluster_), proxyManager_(clientContext_),
                      id_(++CLIENT_ID), random_generator_(id_), uuid_generator_{random_generator_},
                      cp_subsystem_(clientContext_), proxy_session_manager_(clientContext_) {
                const std::shared_ptr<std::string> &name = clientConfig_.getInstanceName();
                if (name.get() != NULL) {
                    instanceName_ = *name;
                } else {
                    std::ostringstream out;
                    out << "hz.client_" << id_;
                    instanceName_ = out.str();
                }

                auto logger_config = clientConfig_.getLoggerConfig();
                logger_ = std::make_shared<logger>(instanceName_, clientConfig_.getClusterName(),
                                                   logger_config.level(), logger_config.handler());

                executionService_ = initExecutionService();

                initalizeNearCacheManager();

                int32_t maxAllowedConcurrentInvocations = clientProperties_.getInteger(
                        clientProperties_.getMaxConcurrentInvocations());
                int64_t backofftimeoutMs = clientProperties_.getLong(
                        clientProperties_.getBackpressureBackoffTimeoutMillis());
                bool isBackPressureEnabled = maxAllowedConcurrentInvocations != INT32_MAX;
                callIdSequence_ = spi::impl::sequence::CallIdFactory::newCallIdSequence(isBackPressureEnabled,
                                                                                       maxAllowedConcurrentInvocations,
                                                                                       backofftimeoutMs);

                std::vector<std::shared_ptr<connection::AddressProvider>> addressProviders = createAddressProviders();

                connectionManager_ = initConnectionManagerService(addressProviders);

                cluster_listener_.reset(new spi::impl::listener::cluster_view_listener(clientContext_));

                partitionService_.reset(new spi::impl::ClientPartitionServiceImpl(clientContext_));

                invocationService_.reset(new spi::impl::ClientInvocationServiceImpl(clientContext_));

                listenerService_ = initListenerService();

                proxyManager_.init();

                lockReferenceIdGenerator_.reset(new impl::ClientLockReferenceIdGenerator());

                statistics_.reset(new statistics::Statistics(clientContext_));
            }

            HazelcastClientInstanceImpl::~HazelcastClientInstanceImpl() = default;

            void HazelcastClientInstanceImpl::start() {
                lifecycleService_.fireLifecycleEvent(LifecycleEvent::STARTING);

                try {
                    if (!lifecycleService_.start()) {
                        lifecycleService_.shutdown();
                        BOOST_THROW_EXCEPTION(exception::IllegalStateException("HazelcastClient",
                                                                               "HazelcastClient could not be started!"));
                    }
                } catch (std::exception &) {
                    lifecycleService_.shutdown();
                    throw;
                }
            }

            ClientConfig &HazelcastClientInstanceImpl::getClientConfig() {
                return clientConfig_;
            }

            Cluster &HazelcastClientInstanceImpl::getCluster() {
                return cluster_;
            }

            boost::uuids::uuid HazelcastClientInstanceImpl::addLifecycleListener(LifecycleListener &&lifecycleListener) {
                return lifecycleService_.addListener(std::move(lifecycleListener));
            }

            bool HazelcastClientInstanceImpl::removeLifecycleListener(const boost::uuids::uuid &registrationId) {
                return lifecycleService_.removeListener(registrationId);
            }

            void HazelcastClientInstanceImpl::shutdown() {
                lifecycleService_.shutdown();
            }

            TransactionContext HazelcastClientInstanceImpl::newTransactionContext() {
                TransactionOptions defaultOptions;
                return newTransactionContext(defaultOptions);
            }

            TransactionContext HazelcastClientInstanceImpl::newTransactionContext(const TransactionOptions &options) {
                return TransactionContext(transactionManager_, options);
            }

            internal::nearcache::NearCacheManager &HazelcastClientInstanceImpl::getNearCacheManager() {
                return *nearCacheManager_;
            }

            serialization::pimpl::SerializationService &HazelcastClientInstanceImpl::getSerializationService() {
                return serializationService_;
            }

            const protocol::ClientExceptionFactory &HazelcastClientInstanceImpl::getExceptionFactory() const {
                return exceptionFactory_;
            }

            std::shared_ptr<spi::impl::listener::listener_service_impl> HazelcastClientInstanceImpl::initListenerService() {
                auto eventThreadCount = clientProperties_.getInteger(clientProperties_.getEventThreadCount());
                return std::make_shared<spi::impl::listener::listener_service_impl>(clientContext_, eventThreadCount);
            }

            std::shared_ptr<spi::impl::ClientExecutionServiceImpl>
            HazelcastClientInstanceImpl::initExecutionService() {
                return std::make_shared<spi::impl::ClientExecutionServiceImpl>(instanceName_, clientProperties_,
                                                                               clientConfig_.getExecutorPoolSize(),
                                                                               lifecycleService_);
            }

            std::shared_ptr<connection::ClientConnectionManagerImpl>
            HazelcastClientInstanceImpl::initConnectionManagerService(
                    const std::vector<std::shared_ptr<connection::AddressProvider>> &addressProviders) {
                config::ClientAwsConfig &awsConfig = clientConfig_.getNetworkConfig().getAwsConfig();
                std::shared_ptr<connection::AddressTranslator> addressTranslator;
                if (awsConfig.isEnabled()) {
                    try {
                        addressTranslator.reset(new aws::impl::AwsAddressTranslator(awsConfig, *logger_));
                    } catch (exception::InvalidConfigurationException &e) {
                        HZ_LOG(*logger_, warning,
                            boost::str(boost::format("Invalid aws configuration! %1%") % e.what())
                        );
                        throw;
                    }
                } else {
                    addressTranslator.reset(new spi::impl::DefaultAddressTranslator());
                }
                return std::make_shared<connection::ClientConnectionManagerImpl>(
                        clientContext_, addressTranslator, addressProviders);

            }

            void HazelcastClientInstanceImpl::on_cluster_restart() {
                HZ_LOG(*logger_, info,
                    "Clearing local state of the client, because of a cluster restart");

                nearCacheManager_->clearAllNearCaches();
                //clear the member list version
                clusterService_.clear_member_list_version();
            }

            std::vector<std::shared_ptr<connection::AddressProvider>>
            HazelcastClientInstanceImpl::createAddressProviders() {
                config::ClientNetworkConfig &networkConfig = getClientConfig().getNetworkConfig();
                config::ClientAwsConfig &awsConfig = networkConfig.getAwsConfig();
                std::vector<std::shared_ptr<connection::AddressProvider>> addressProviders;

                if (awsConfig.isEnabled()) {
                    int awsMemberPort = clientProperties_.getInteger(clientProperties_.getAwsMemberPort());
                    if (awsMemberPort < 0 || awsMemberPort > 65535) {
                        throw (exception::ExceptionBuilder<exception::InvalidConfigurationException>(
                                "HazelcastClientInstanceImpl::createAddressProviders") << "Configured aws member port "
                                                                                       << awsMemberPort
                                                                                       << " is not a valid port number. It should be between 0-65535 inclusive.").build();
                    }
                    addressProviders.push_back(std::shared_ptr<connection::AddressProvider>(
                            new spi::impl::AwsAddressProvider(awsConfig, awsMemberPort, *logger_)));
                }

                addressProviders.push_back(std::shared_ptr<connection::AddressProvider>(
                        new spi::impl::DefaultAddressProvider(networkConfig, addressProviders.empty())));

                return addressProviders;
            }

            const std::string &HazelcastClientInstanceImpl::getName() const {
                return instanceName_;
            }

            spi::LifecycleService &HazelcastClientInstanceImpl::getLifecycleService() {
                return lifecycleService_;
            }

            const std::shared_ptr<ClientLockReferenceIdGenerator> &
            HazelcastClientInstanceImpl::getLockReferenceIdGenerator() const {
                return lockReferenceIdGenerator_;
            }

            spi::ProxyManager &HazelcastClientInstanceImpl::getProxyManager() {
                return proxyManager_;
            }

            void HazelcastClientInstanceImpl::initalizeNearCacheManager() {
                nearCacheManager_.reset(
                        new internal::nearcache::NearCacheManager(executionService_, serializationService_, *logger_));
            }

            Client HazelcastClientInstanceImpl::getLocalEndpoint() const {
                return clusterService_.getLocalClient();
            }

            template<>
            std::shared_ptr<IMap> HazelcastClientInstanceImpl::getDistributedObject(const std::string& name) {
                auto nearCacheConfig = clientConfig_.getNearCacheConfig(name);
                if (nearCacheConfig) {
                    return proxyManager_.getOrCreateProxy<map::NearCachedClientMapProxy<serialization::pimpl::Data, serialization::pimpl::Data>>(
                            IMap::SERVICE_NAME, name);
                } else {
                    return proxyManager_.getOrCreateProxy<IMap>(IMap::SERVICE_NAME, name);
                }
            }

            const std::shared_ptr<logger> &HazelcastClientInstanceImpl::getLogger() const {
                return logger_;
            }

            boost::uuids::uuid HazelcastClientInstanceImpl::random_uuid() {
                std::lock_guard<std::mutex> g(uuid_generator_lock_);
                return uuid_generator_();
            }

            cp::cp_subsystem &HazelcastClientInstanceImpl::get_cp_subsystem() {
                return cp_subsystem_;
            }

            BaseEventHandler::~BaseEventHandler() = default;

            BaseEventHandler::BaseEventHandler() : logger_(nullptr) {}

            void BaseEventHandler::setLogger(logger *lg) {
                BaseEventHandler::logger_ = lg;
            }

            logger *BaseEventHandler::getLogger() const {
                return logger_;
            }

        }

        LoadBalancer::~LoadBalancer() = default;

        const int Address::ID = cluster::impl::ADDRESS;

        const byte Address::IPV4 = 4;
        const byte Address::IPV6 = 6;

        Address::Address() : host_("localhost"), type_(IPV4), scopeId_(0) {
        }

        Address::Address(const std::string &url, int port)
                : host_(url), port_(port), type_(IPV4), scopeId_(0) {
        }

        Address::Address(const std::string &hostname, int port, unsigned long scopeId) : host_(hostname), port_(port),
                                                                                         type_(IPV6), scopeId_(scopeId) {
        }

        bool Address::operator==(const Address &rhs) const {
            return rhs.port_ == port_ && rhs.type_ == type_ && 0 == rhs.host_.compare(host_);
        }

        bool Address::operator!=(const Address &rhs) const {
            return !(*this == rhs);
        }

        int Address::getPort() const {
            return port_;
        }

        const std::string &Address::getHost() const {
            return host_;
        }

        bool Address::operator<(const Address &rhs) const {
            if (host_ < rhs.host_) {
                return true;
            }
            if (rhs.host_ < host_) {
                return false;
            }
            if (port_ < rhs.port_) {
                return true;
            }
            if (rhs.port_ < port_) {
                return false;
            }
            return type_ < rhs.type_;
        }

        bool Address::isIpV4() const {
            return type_ == IPV4;
        }

        unsigned long Address::getScopeId() const {
            return scopeId_;
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
                out.write<int32_t>(object.port_);
                out.write<byte>(object.type_);
                out.write(object.host_);
            }

            Address hz_serializer<Address>::readData(ObjectDataInput &in) {
                Address object;
                object.port_ = in.read<int32_t>();
                object.type_ = in.read<byte>();
                object.host_ = in.read<std::string>();
                return object;
            }
        }

        IExecutorService::IExecutorService(const std::string &name, spi::ClientContext *context) : ProxyImpl(
                SERVICE_NAME, name, context), consecutiveSubmits_(0), lastSubmitTime_(0) {
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
        IExecutorService::invokeOnTarget(protocol::ClientMessage &&request, boost::uuids::uuid target) {
            try {
                std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), target);
                return std::make_pair(clientInvocation->invoke(), clientInvocation);
            } catch (exception::IException &) {
                util::ExceptionUtil::rethrow(std::current_exception());
            }
            return std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>();
        }

        std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
        IExecutorService::invokeOnPartitionOwner(protocol::ClientMessage &&request, int partitionId) {
            try {
                std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), partitionId);
                return std::make_pair(clientInvocation->invoke(), clientInvocation);
            } catch (exception::IException &) {
                util::ExceptionUtil::rethrow(std::current_exception());
            }
            return std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>();
        }

        bool IExecutorService::isSyncComputation(bool preventSync) {
            int64_t now = util::currentTimeMillis();

            int64_t last = lastSubmitTime_;
            lastSubmitTime_ = now;

            if (last + MIN_TIME_RESOLUTION_OF_CONSECUTIVE_SUBMITS < now) {
                consecutiveSubmits_ = 0;
                return false;
            }

            return !preventSync && (consecutiveSubmits_++ % MAX_CONSECUTIVE_SUBMITS == 0);
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
            auto &partitionService = getContext().getPartitionService();
            return rand() % partitionService.getPartitionCount();
        }

        void IExecutorService::shutdown() {
            auto request = protocol::codec::executorservice_shutdown_encode(
                    getName());
            invoke(request);
        }

        boost::future<bool> IExecutorService::isShutdown() {
            auto request = protocol::codec::executorservice_isshutdown_encode(
                    getName());
            return invokeAndGetFuture<bool>(
                    request);
        }

        boost::future<bool> IExecutorService::isTerminated() {
            return isShutdown();
        }

        const std::string ClientProperties::PROP_HEARTBEAT_TIMEOUT = "hazelcast_client_heartbeat_timeout";
        const std::string ClientProperties::PROP_HEARTBEAT_TIMEOUT_DEFAULT = "60000";
        const std::string ClientProperties::PROP_HEARTBEAT_INTERVAL = "hazelcast_client_heartbeat_interval";
        const std::string ClientProperties::PROP_HEARTBEAT_INTERVAL_DEFAULT = "5000";
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
                : name_(name), defaultValue_(defaultValue) {
        }

        const std::string &ClientProperty::getName() const {
            return name_;
        }

        const std::string &ClientProperty::getDefaultValue() const {
            return defaultValue_;
        }

        const char *ClientProperty::getSystemProperty() const {
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            #pragma warning(push)
#pragma warning(disable: 4996) //for 'getenv': This function or variable may be unsafe.
#endif
            return std::getenv(name_.c_str());
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            #pragma warning(pop)
#endif
        }

        ClientProperties::ClientProperties(const std::unordered_map<std::string, std::string> &properties)
                : heartbeatTimeout_(PROP_HEARTBEAT_TIMEOUT, PROP_HEARTBEAT_TIMEOUT_DEFAULT),
                  heartbeatInterval_(PROP_HEARTBEAT_INTERVAL, PROP_HEARTBEAT_INTERVAL_DEFAULT),
                  retryCount_(PROP_REQUEST_RETRY_COUNT, PROP_REQUEST_RETRY_COUNT_DEFAULT),
                  retryWaitTime_(PROP_REQUEST_RETRY_WAIT_TIME, PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT),
                  awsMemberPort_(PROP_AWS_MEMBER_PORT, PROP_AWS_MEMBER_PORT_DEFAULT),
                  cleanResourcesPeriod_(CLEAN_RESOURCES_PERIOD_MILLIS,
                                       CLEAN_RESOURCES_PERIOD_MILLIS_DEFAULT),
                  invocationRetryPauseMillis_(INVOCATION_RETRY_PAUSE_MILLIS,
                                             INVOCATION_RETRY_PAUSE_MILLIS_DEFAULT),
                  invocationTimeoutSeconds_(INVOCATION_TIMEOUT_SECONDS,
                                           INVOCATION_TIMEOUT_SECONDS_DEFAULT),
                  eventThreadCount_(EVENT_THREAD_COUNT, EVENT_THREAD_COUNT_DEFAULT),
                  internalExecutorPoolSize_(INTERNAL_EXECUTOR_POOL_SIZE,
                                           INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT),
                  shuffleMemberList_(SHUFFLE_MEMBER_LIST, SHUFFLE_MEMBER_LIST_DEFAULT),
                  maxConcurrentInvocations_(MAX_CONCURRENT_INVOCATIONS,
                                           MAX_CONCURRENT_INVOCATIONS_DEFAULT),
                  backpressureBackoffTimeoutMillis_(BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS,
                                                   BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS_DEFAULT),
                  statisticsEnabled_(STATISTICS_ENABLED, STATISTICS_ENABLED_DEFAULT),
                  statisticsPeriodSeconds_(STATISTICS_PERIOD_SECONDS, STATISTICS_PERIOD_SECONDS_DEFAULT),
                  ioThreadCount_(IO_THREAD_COUNT, IO_THREAD_COUNT_DEFAULT),
                  responseExecutorThreadCount_(RESPONSE_EXECUTOR_THREAD_COUNT, RESPONSE_EXECUTOR_THREAD_COUNT_DEFAULT),
                  backup_timeout_millis_(OPERATION_BACKUP_TIMEOUT_MILLIS, OPERATION_BACKUP_TIMEOUT_MILLIS_DEFAULT),
                  fail_on_indeterminate_state_(FAIL_ON_INDETERMINATE_OPERATION_STATE, FAIL_ON_INDETERMINATE_OPERATION_STATE_DEFAULT),
                  propertiesMap_(properties) {
        }

        const ClientProperty &ClientProperties::getHeartbeatTimeout() const {
            return heartbeatTimeout_;
        }

        const ClientProperty &ClientProperties::getHeartbeatInterval() const {
            return heartbeatInterval_;
        }

        const ClientProperty &ClientProperties::getAwsMemberPort() const {
            return awsMemberPort_;
        }

        const ClientProperty &ClientProperties::getCleanResourcesPeriodMillis() const {
            return cleanResourcesPeriod_;
        }

        const ClientProperty &ClientProperties::getInvocationRetryPauseMillis() const {
            return invocationRetryPauseMillis_;
        }

        const ClientProperty &ClientProperties::getInvocationTimeoutSeconds() const {
            return invocationTimeoutSeconds_;
        }

        const ClientProperty &ClientProperties::getEventThreadCount() const {
            return eventThreadCount_;
        }

        const ClientProperty &ClientProperties::getInternalExecutorPoolSize() const {
            return internalExecutorPoolSize_;
        }

        const ClientProperty &ClientProperties::getShuffleMemberList() const {
            return shuffleMemberList_;
        }

        const ClientProperty &ClientProperties::getMaxConcurrentInvocations() const {
            return maxConcurrentInvocations_;
        }

        const ClientProperty &ClientProperties::getBackpressureBackoffTimeoutMillis() const {
            return backpressureBackoffTimeoutMillis_;
        }

        const ClientProperty &ClientProperties::getStatisticsEnabled() const {
            return statisticsEnabled_;
        }

        const ClientProperty &ClientProperties::getStatisticsPeriodSeconds() const {
            return statisticsPeriodSeconds_;
        }

        const ClientProperty &ClientProperties::getIOThreadCount() const {
            return ioThreadCount_;
        }

        std::string ClientProperties::getString(const ClientProperty &property) const {
            std::unordered_map<std::string, std::string>::const_iterator valueIt = propertiesMap_.find(property.getName());
            if (valueIt != propertiesMap_.end()) {
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
            return responseExecutorThreadCount_;
        }

        const ClientProperty &ClientProperties::backup_timeout_millis() const {
            return backup_timeout_millis_;
        }

        const ClientProperty &ClientProperties::fail_on_indeterminate_state() const {
            return fail_on_indeterminate_state_;
        }

        namespace exception {
            IException::IException(const std::string &exceptionName, const std::string &source,
                                   const std::string &message, const std::string &details, int32_t errorNo,
                                   std::exception_ptr cause, bool isRuntime, bool retryable)
                    : src_(source), msg_(message), details_(details), errorCode_(errorNo), cause_(cause),
                    runtimeException_(isRuntime), retryable_(retryable), report_((boost::format(
                            "%1% {%2%. Error code:%3%, Details:%4%.} at %5%.") % exceptionName % message % errorNo %
                                                    details % source).str()) {
            }

            IException::~IException() noexcept = default;

            char const *IException::what() const noexcept {
                return report_.c_str();
            }

            const std::string &IException::getSource() const {
                return src_;
            }

            const std::string &IException::getMessage() const {
                return msg_;
            }

            std::ostream &operator<<(std::ostream &os, const IException &exception) {
                os << exception.what();
                return os;
            }

            const std::string &IException::getDetails() const {
                return details_;
            }

            int32_t IException::getErrorCode() const {
                return errorCode_;
            }

            bool IException::isRuntimeException() const {
                return runtimeException_;
            }

            bool IException::isRetryable() const {
                return retryable_;
            }

            IException::IException() = default;

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message,
                                                                     const std::string &details,
                                                                     std::exception_ptr cause)
                    : RetryableHazelcastException(
                    "RetryableHazelcastException", protocol::RETRYABLE_HAZELCAST, source, message, details, cause, true,
                    true) {}

            RetryableHazelcastException::RetryableHazelcastException(const std::string &errorName, int32_t errorCode,
                                                                     const std::string &source,
                                                                     const std::string &message,
                                                                     const std::string &details, std::exception_ptr cause,
                                                                     bool runtime, bool retryable) : HazelcastException(errorName,
                                                                                                          errorCode,
                                                                                                          source,
                                                                                                          message,
                                                                                                          details,
                                                                                                          cause,
                                                                                                          runtime,
                                                                                                          retryable) {}

            MemberLeftException::MemberLeftException(const std::string &source, const std::string &message,
                                                     const std::string &details, std::exception_ptr cause)
                    : ExecutionException("MemberLeftException", protocol::MEMBER_LEFT, source, message, details,
                                         cause, false,true) {}

            ConsistencyLostException::ConsistencyLostException(const std::string &source, const std::string &message,
                                                               const std::string &details, std::exception_ptr cause)
                    : HazelcastException("ConsistencyLostException", protocol::CONSISTENCY_LOST_EXCEPTION, source, message,
                                         details, cause, true,false) {}
        }
    }
}

namespace std {
    std::size_t hash<hazelcast::client::Address>::operator()(const hazelcast::client::Address &address) const noexcept {
        return std::hash<std::string>()(address.getHost()) + std::hash<int>()(address.getPort()) +
               std::hash<unsigned long>()(address.type_);
    }
}
