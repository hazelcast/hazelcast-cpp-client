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

#ifndef HAZELCAST_VERSION
#define HAZELCAST_VERSION "NOT_FOUND"
#endif

INITIALIZE_EASYLOGGINGPP

namespace hazelcast {
    namespace client {
        HazelcastClient::HazelcastClient() : client_impl_(new impl::HazelcastClientInstanceImpl(ClientConfig())) {
            client_impl_->start();
        }

        HazelcastClient::HazelcastClient(const ClientConfig &config) : client_impl_(
                new impl::HazelcastClientInstanceImpl(config)) {
            client_impl_->start();
        }

        const std::string &HazelcastClient::getName() const {
            return client_impl_->getName();
        }

        ClientConfig &HazelcastClient::getClientConfig() {
            return client_impl_->getClientConfig();
        }

        TransactionContext HazelcastClient::newTransactionContext() {
            return client_impl_->newTransactionContext();
        }

        TransactionContext HazelcastClient::newTransactionContext(const TransactionOptions &options) {
            return client_impl_->newTransactionContext(options);
        }

        Cluster &HazelcastClient::getCluster() {
            return client_impl_->getCluster();
        }

        boost::uuids::uuid HazelcastClient::addLifecycleListener(LifecycleListener &&lifecycleListener) {
            return client_impl_->addLifecycleListener(std::move(lifecycleListener));
        }

        bool HazelcastClient::removeLifecycleListener(const boost::uuids::uuid &registrationId) {
            return client_impl_->removeLifecycleListener(registrationId);
        }

        void HazelcastClient::shutdown() {
            client_impl_->shutdown();
        }

        spi::LifecycleService &HazelcastClient::getLifecycleService() {
            return client_impl_->getLifecycleService();
        }

        Client HazelcastClient::getLocalEndpoint() const {
            return client_impl_->getLocalEndpoint();
        }

        HazelcastClient::~HazelcastClient() {
            client_impl_->shutdown();
        }

        cp::cp_subsystem &HazelcastClient::get_cp_subsystem() {
            return client_impl_->get_cp_subsystem();
        }

        namespace impl {
            std::atomic<int32_t> HazelcastClientInstanceImpl::CLIENT_ID(0);

            HazelcastClientInstanceImpl::HazelcastClientInstanceImpl(const ClientConfig &config)
                    : client_config_(config), client_properties_(config.getProperties()),
                      client_context_(*this),
                      serialization_service_(client_config_.getSerializationConfig()), cluster_service_(client_context_),
                      transaction_manager_(client_context_), cluster_(cluster_service_),
                      lifecycle_service_(client_context_, client_config_.getLifecycleListeners(),
                                       client_config_.getLoadBalancer(), cluster_), proxy_manager_(client_context_),
                      id_(++CLIENT_ID), random_generator_(id_), uuid_generator_{random_generator_},
                      cp_subsystem_(client_context_) {
                const std::shared_ptr<std::string> &name = client_config_.getInstanceName();
                if (name.get() != NULL) {
                    instance_name_ = *name;
                } else {
                    std::ostringstream out;
                    out << "hz.client_" << id_;
                    instance_name_ = out.str();
                }

                logger_.reset(new util::ILogger(instance_name_, client_config_.getGroupConfig().getName(), HAZELCAST_VERSION,
                                               client_config_.getLoggerConfig()));

                execution_service_ = initExecutionService();

                initalizeNearCacheManager();

                int32_t maxAllowedConcurrentInvocations = client_properties_.getInteger(
                        client_properties_.getMaxConcurrentInvocations());
                int64_t backofftimeoutMs = client_properties_.getLong(
                        client_properties_.getBackpressureBackoffTimeoutMillis());
                bool isBackPressureEnabled = maxAllowedConcurrentInvocations != INT32_MAX;
                call_id_sequence_ = spi::impl::sequence::CallIdFactory::newCallIdSequence(isBackPressureEnabled,
                                                                                       maxAllowedConcurrentInvocations,
                                                                                       backofftimeoutMs);

                std::vector<std::shared_ptr<connection::AddressProvider>> addressProviders = createAddressProviders();

                connection_manager_ = initConnectionManagerService(addressProviders);

                cluster_listener_.reset(new spi::impl::listener::cluster_view_listener(client_context_));

                partition_service_.reset(new spi::impl::ClientPartitionServiceImpl(client_context_));

                invocation_service_.reset(new spi::impl::ClientInvocationServiceImpl(client_context_));

                listener_service_ = initListenerService();

                proxy_manager_.init();

                lock_reference_id_generator_.reset(new impl::ClientLockReferenceIdGenerator());

                statistics_.reset(new statistics::Statistics(client_context_));
            }

            HazelcastClientInstanceImpl::~HazelcastClientInstanceImpl() = default;

            void HazelcastClientInstanceImpl::start() {
                startLogger();

                lifecycle_service_.fireLifecycleEvent(LifecycleEvent::STARTING);

                try {
                    if (!lifecycle_service_.start()) {
                        lifecycle_service_.shutdown();
                        BOOST_THROW_EXCEPTION(exception::IllegalStateException("HazelcastClient",
                                                                               "HazelcastClient could not be started!"));
                    }
                } catch (std::exception &) {
                    lifecycle_service_.shutdown();
                    throw;
                }
            }

            void HazelcastClientInstanceImpl::startLogger() {
                try {
                    if (!logger_->start()) {
                        throw (exception::ExceptionBuilder<exception::IllegalStateException>(
                                "HazelcastClientInstanceImpl::initLogger")
                                << "Could not start logger for instance " << instance_name_).build();
                    }
                } catch (std::invalid_argument &ia) {
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalStateException("HazelcastClientInstanceImpl::initLogger", ia.what()));
                }
            }

            ClientConfig &HazelcastClientInstanceImpl::getClientConfig() {
                return client_config_;
            }

            Cluster &HazelcastClientInstanceImpl::getCluster() {
                return cluster_;
            }

            boost::uuids::uuid HazelcastClientInstanceImpl::addLifecycleListener(LifecycleListener &&lifecycleListener) {
                return lifecycle_service_.addListener(std::move(lifecycleListener));
            }

            bool HazelcastClientInstanceImpl::removeLifecycleListener(const boost::uuids::uuid &registrationId) {
                return lifecycle_service_.removeListener(registrationId);
            }

            void HazelcastClientInstanceImpl::shutdown() {
                lifecycle_service_.shutdown();
            }

            TransactionContext HazelcastClientInstanceImpl::newTransactionContext() {
                TransactionOptions defaultOptions;
                return newTransactionContext(defaultOptions);
            }

            TransactionContext HazelcastClientInstanceImpl::newTransactionContext(const TransactionOptions &options) {
                return TransactionContext(transaction_manager_, options);
            }

            internal::nearcache::NearCacheManager &HazelcastClientInstanceImpl::getNearCacheManager() {
                return *near_cache_manager_;
            }

            serialization::pimpl::SerializationService &HazelcastClientInstanceImpl::getSerializationService() {
                return serialization_service_;
            }

            const protocol::ClientExceptionFactory &HazelcastClientInstanceImpl::getExceptionFactory() const {
                return exception_factory_;
            }

            std::shared_ptr<spi::impl::listener::listener_service_impl> HazelcastClientInstanceImpl::initListenerService() {
                auto eventThreadCount = client_properties_.getInteger(client_properties_.getEventThreadCount());
                return std::make_shared<spi::impl::listener::listener_service_impl>(client_context_, eventThreadCount);
            }

            std::shared_ptr<spi::impl::ClientExecutionServiceImpl>
            HazelcastClientInstanceImpl::initExecutionService() {
                return std::make_shared<spi::impl::ClientExecutionServiceImpl>(instance_name_, client_properties_,
                                                                               client_config_.getExecutorPoolSize(),
                                                                               lifecycle_service_);
            }

            std::shared_ptr<connection::ClientConnectionManagerImpl>
            HazelcastClientInstanceImpl::initConnectionManagerService(
                    const std::vector<std::shared_ptr<connection::AddressProvider>> &addressProviders) {
                config::ClientAwsConfig &awsConfig = client_config_.getNetworkConfig().getAwsConfig();
                std::shared_ptr<connection::AddressTranslator> addressTranslator;
                if (awsConfig.isEnabled()) {
                    try {
                        addressTranslator.reset(new aws::impl::AwsAddressTranslator(awsConfig, *logger_));
                    } catch (exception::InvalidConfigurationException &e) {
                        logger_->warning(std::string("Invalid aws configuration! ") + e.what());
                        throw;
                    }
                } else {
                    addressTranslator.reset(new spi::impl::DefaultAddressTranslator());
                }
                return std::make_shared<connection::ClientConnectionManagerImpl>(
                        client_context_, addressTranslator, addressProviders);

            }

            void HazelcastClientInstanceImpl::on_cluster_restart() {
                logger_->info("Clearing local state of the client, because of a cluster restart");

                near_cache_manager_->clearAllNearCaches();
                //clear the member list version
                cluster_service_.clear_member_list_version();
            }

            std::vector<std::shared_ptr<connection::AddressProvider>>
            HazelcastClientInstanceImpl::createAddressProviders() {
                config::ClientNetworkConfig &networkConfig = getClientConfig().getNetworkConfig();
                config::ClientAwsConfig &awsConfig = networkConfig.getAwsConfig();
                std::vector<std::shared_ptr<connection::AddressProvider>> addressProviders;

                if (awsConfig.isEnabled()) {
                    int awsMemberPort = client_properties_.getInteger(client_properties_.getAwsMemberPort());
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
                return instance_name_;
            }

            spi::LifecycleService &HazelcastClientInstanceImpl::getLifecycleService() {
                return lifecycle_service_;
            }

            const std::shared_ptr<ClientLockReferenceIdGenerator> &
            HazelcastClientInstanceImpl::getLockReferenceIdGenerator() const {
                return lock_reference_id_generator_;
            }

            spi::ProxyManager &HazelcastClientInstanceImpl::getProxyManager() {
                return proxy_manager_;
            }

            void HazelcastClientInstanceImpl::initalizeNearCacheManager() {
                near_cache_manager_.reset(
                        new internal::nearcache::NearCacheManager(execution_service_, serialization_service_, *logger_));
            }

            Client HazelcastClientInstanceImpl::getLocalEndpoint() const {
                return cluster_service_.getLocalClient();
            }

            template<>
            std::shared_ptr<IMap> HazelcastClientInstanceImpl::getDistributedObject(const std::string& name) {
                auto nearCacheConfig = client_config_.getNearCacheConfig<serialization::pimpl::Data, serialization::pimpl::Data>(
                        name);
                if (nearCacheConfig) {
                    return proxy_manager_.getOrCreateProxy<map::NearCachedClientMapProxy<serialization::pimpl::Data, serialization::pimpl::Data>>(
                            IMap::SERVICE_NAME, name);
                } else {
                    return proxy_manager_.getOrCreateProxy<IMap>(IMap::SERVICE_NAME, name);
                }
            }

            const std::shared_ptr<util::ILogger> &HazelcastClientInstanceImpl::getLogger() const {
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

            BaseEventHandler::BaseEventHandler() : logger(NULL) {}

            void BaseEventHandler::setLogger(util::ILogger *iLogger) {
                BaseEventHandler::logger = iLogger;
            }

            util::ILogger *BaseEventHandler::getLogger() const {
                return logger;
            }

        }

        LoadBalancer::~LoadBalancer() = default;

        const int Address::ID = cluster::impl::ADDRESS;

        const byte Address::IPV4 = 4;
        const byte Address::IPV6 = 6;

        Address::Address() : host_("localhost"), type_(IPV4), scope_id_(0) {
        }

        Address::Address(const std::string &url, int port)
                : host_(url), port_(port), type_(IPV4), scope_id_(0) {
        }

        Address::Address(const std::string &hostname, int port, unsigned long scopeId) : host_(hostname), port_(port),
                                                                                         type_(IPV6), scope_id_(scopeId) {
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
            return scope_id_;
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
                SERVICE_NAME, name, context), consecutive_submits_(0), last_submit_time_(0) {
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

            int64_t last = last_submit_time_;
            last_submit_time_ = now;

            if (last + MIN_TIME_RESOLUTION_OF_CONSECUTIVE_SUBMITS < now) {
                consecutive_submits_ = 0;
                return false;
            }

            return !preventSync && (consecutive_submits_++ % MAX_CONSECUTIVE_SUBMITS == 0);
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
                : name_(name), default_value_(defaultValue) {
        }

        const std::string &ClientProperty::getName() const {
            return name_;
        }

        const std::string &ClientProperty::getDefaultValue() const {
            return default_value_;
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
                : heartbeat_timeout_(PROP_HEARTBEAT_TIMEOUT, PROP_HEARTBEAT_TIMEOUT_DEFAULT),
                  heartbeat_interval_(PROP_HEARTBEAT_INTERVAL, PROP_HEARTBEAT_INTERVAL_DEFAULT),
                  retry_count_(PROP_REQUEST_RETRY_COUNT, PROP_REQUEST_RETRY_COUNT_DEFAULT),
                  retry_wait_time_(PROP_REQUEST_RETRY_WAIT_TIME, PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT),
                  aws_member_port_(PROP_AWS_MEMBER_PORT, PROP_AWS_MEMBER_PORT_DEFAULT),
                  clean_resources_period_(CLEAN_RESOURCES_PERIOD_MILLIS,
                                       CLEAN_RESOURCES_PERIOD_MILLIS_DEFAULT),
                  invocation_retry_pause_millis_(INVOCATION_RETRY_PAUSE_MILLIS,
                                             INVOCATION_RETRY_PAUSE_MILLIS_DEFAULT),
                  invocation_timeout_seconds_(INVOCATION_TIMEOUT_SECONDS,
                                           INVOCATION_TIMEOUT_SECONDS_DEFAULT),
                  event_thread_count_(EVENT_THREAD_COUNT, EVENT_THREAD_COUNT_DEFAULT),
                  internal_executor_pool_size_(INTERNAL_EXECUTOR_POOL_SIZE,
                                           INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT),
                  shuffle_member_list_(SHUFFLE_MEMBER_LIST, SHUFFLE_MEMBER_LIST_DEFAULT),
                  max_concurrent_invocations_(MAX_CONCURRENT_INVOCATIONS,
                                           MAX_CONCURRENT_INVOCATIONS_DEFAULT),
                  backpressure_backoff_timeout_millis_(BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS,
                                                   BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS_DEFAULT),
                  statistics_enabled_(STATISTICS_ENABLED, STATISTICS_ENABLED_DEFAULT),
                  statistics_period_seconds_(STATISTICS_PERIOD_SECONDS, STATISTICS_PERIOD_SECONDS_DEFAULT),
                  io_thread_count_(IO_THREAD_COUNT, IO_THREAD_COUNT_DEFAULT),
                  response_executor_thread_count_(RESPONSE_EXECUTOR_THREAD_COUNT, RESPONSE_EXECUTOR_THREAD_COUNT_DEFAULT),
                  properties_map_(properties) {
        }

        const ClientProperty &ClientProperties::getHeartbeatTimeout() const {
            return heartbeat_timeout_;
        }

        const ClientProperty &ClientProperties::getHeartbeatInterval() const {
            return heartbeat_interval_;
        }

        const ClientProperty &ClientProperties::getAwsMemberPort() const {
            return aws_member_port_;
        }

        const ClientProperty &ClientProperties::getCleanResourcesPeriodMillis() const {
            return clean_resources_period_;
        }

        const ClientProperty &ClientProperties::getInvocationRetryPauseMillis() const {
            return invocation_retry_pause_millis_;
        }

        const ClientProperty &ClientProperties::getInvocationTimeoutSeconds() const {
            return invocation_timeout_seconds_;
        }

        const ClientProperty &ClientProperties::getEventThreadCount() const {
            return event_thread_count_;
        }

        const ClientProperty &ClientProperties::getInternalExecutorPoolSize() const {
            return internal_executor_pool_size_;
        }

        const ClientProperty &ClientProperties::getShuffleMemberList() const {
            return shuffle_member_list_;
        }

        const ClientProperty &ClientProperties::getMaxConcurrentInvocations() const {
            return max_concurrent_invocations_;
        }

        const ClientProperty &ClientProperties::getBackpressureBackoffTimeoutMillis() const {
            return backpressure_backoff_timeout_millis_;
        }

        const ClientProperty &ClientProperties::getStatisticsEnabled() const {
            return statistics_enabled_;
        }

        const ClientProperty &ClientProperties::getStatisticsPeriodSeconds() const {
            return statistics_period_seconds_;
        }

        const ClientProperty &ClientProperties::getIOThreadCount() const {
            return io_thread_count_;
        }

        std::string ClientProperties::getString(const ClientProperty &property) const {
            std::unordered_map<std::string, std::string>::const_iterator valueIt = properties_map_.find(property.getName());
            if (valueIt != properties_map_.end()) {
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
            return response_executor_thread_count_;
        }

        namespace exception {
            IException::IException(const std::string &exceptionName, const std::string &source,
                                   const std::string &message, const std::string &details, int32_t errorNo,
                                   std::exception_ptr cause, bool isRuntime, bool retryable)
                    : src(source), msg(message), details(details), errorCode(errorNo), cause_(cause),
                    runtimeException(isRuntime), retryable(retryable), report((boost::format(
                            "%1% {%2%. Error code:%3%, Details:%4%.} at %5%.") % exceptionName % message % errorNo %
                                                    details % source).str()) {
            }

            IException::~IException() noexcept = default;

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
