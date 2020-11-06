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

        const std::string &HazelcastClient::get_name() const {
            return clientImpl_->get_name();
        }

        ClientConfig &HazelcastClient::get_client_config() {
            return clientImpl_->get_client_config();
        }

        TransactionContext HazelcastClient::new_transaction_context() {
            return clientImpl_->new_transaction_context();
        }

        TransactionContext HazelcastClient::new_transaction_context(const TransactionOptions &options) {
            return clientImpl_->new_transaction_context(options);
        }

        Cluster &HazelcastClient::get_cluster() {
            return clientImpl_->get_cluster();
        }

        boost::uuids::uuid HazelcastClient::add_lifecycle_listener(LifecycleListener &&lifecycle_listener) {
            return clientImpl_->add_lifecycle_listener(std::move(lifecycle_listener));
        }

        bool HazelcastClient::remove_lifecycle_listener(const boost::uuids::uuid &registration_id) {
            return clientImpl_->remove_lifecycle_listener(registration_id);
        }

        void HazelcastClient::shutdown() {
            clientImpl_->shutdown();
        }

        spi::LifecycleService &HazelcastClient::get_lifecycle_service() {
            return clientImpl_->get_lifecycle_service();
        }

        Client HazelcastClient::get_local_endpoint() const {
            return clientImpl_->get_local_endpoint();
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
                    : clientConfig_(config), clientProperties_(config.get_properties()),
                      clientContext_(*this),
                      serializationService_(clientConfig_.get_serialization_config()), clusterService_(clientContext_),
                      transactionManager_(clientContext_), cluster_(clusterService_),
                      lifecycleService_(clientContext_, clientConfig_.get_lifecycle_listeners(),
                                       clientConfig_.get_load_balancer(), cluster_), proxyManager_(clientContext_),
                      id_(++CLIENT_ID), random_generator_(id_), uuid_generator_{random_generator_},
                      cp_subsystem_(clientContext_), proxy_session_manager_(clientContext_) {
                const std::shared_ptr<std::string> &name = clientConfig_.get_instance_name();
                if (name.get() != NULL) {
                    instanceName_ = *name;
                } else {
                    std::ostringstream out;
                    out << "hz.client_" << id_;
                    instanceName_ = out.str();
                }

                auto logger_config = clientConfig_.get_logger_config();
                logger_ = std::make_shared<logger>(instanceName_, clientConfig_.get_cluster_name(),
                                                   logger_config.level(), logger_config.handler());

                executionService_ = init_execution_service();

                initalize_near_cache_manager();

                int32_t maxAllowedConcurrentInvocations = clientProperties_.get_integer(
                        clientProperties_.get_max_concurrent_invocations());
                int64_t backofftimeoutMs = clientProperties_.get_long(
                        clientProperties_.get_backpressure_backoff_timeout_millis());
                bool isBackPressureEnabled = maxAllowedConcurrentInvocations != INT32_MAX;
                callIdSequence_ = spi::impl::sequence::CallIdFactory::new_call_id_sequence(isBackPressureEnabled,
                                                                                       maxAllowedConcurrentInvocations,
                                                                                       backofftimeoutMs);

                std::vector<std::shared_ptr<connection::AddressProvider>> addressProviders = create_address_providers();

                connectionManager_ = init_connection_manager_service(addressProviders);

                cluster_listener_.reset(new spi::impl::listener::cluster_view_listener(clientContext_));

                partitionService_.reset(new spi::impl::ClientPartitionServiceImpl(clientContext_));

                invocationService_.reset(new spi::impl::ClientInvocationServiceImpl(clientContext_));

                listenerService_ = init_listener_service();

                proxyManager_.init();

                lockReferenceIdGenerator_.reset(new impl::ClientLockReferenceIdGenerator());

                statistics_.reset(new statistics::Statistics(clientContext_));
            }

            HazelcastClientInstanceImpl::~HazelcastClientInstanceImpl() = default;

            void HazelcastClientInstanceImpl::start() {
                lifecycleService_.fire_lifecycle_event(LifecycleEvent::STARTING);

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

            ClientConfig &HazelcastClientInstanceImpl::get_client_config() {
                return clientConfig_;
            }

            Cluster &HazelcastClientInstanceImpl::get_cluster() {
                return cluster_;
            }

            boost::uuids::uuid HazelcastClientInstanceImpl::add_lifecycle_listener(LifecycleListener &&lifecycle_listener) {
                return lifecycleService_.add_listener(std::move(lifecycle_listener));
            }

            bool HazelcastClientInstanceImpl::remove_lifecycle_listener(const boost::uuids::uuid &registration_id) {
                return lifecycleService_.remove_listener(registration_id);
            }

            void HazelcastClientInstanceImpl::shutdown() {
                lifecycleService_.shutdown();
            }

            TransactionContext HazelcastClientInstanceImpl::new_transaction_context() {
                TransactionOptions defaultOptions;
                return new_transaction_context(defaultOptions);
            }

            TransactionContext HazelcastClientInstanceImpl::new_transaction_context(const TransactionOptions &options) {
                return TransactionContext(transactionManager_, options);
            }

            internal::nearcache::NearCacheManager &HazelcastClientInstanceImpl::get_near_cache_manager() {
                return *nearCacheManager_;
            }

            serialization::pimpl::SerializationService &HazelcastClientInstanceImpl::get_serialization_service() {
                return serializationService_;
            }

            const protocol::ClientExceptionFactory &HazelcastClientInstanceImpl::get_exception_factory() const {
                return exceptionFactory_;
            }

            std::shared_ptr<spi::impl::listener::listener_service_impl> HazelcastClientInstanceImpl::init_listener_service() {
                auto eventThreadCount = clientProperties_.get_integer(clientProperties_.get_event_thread_count());
                return std::make_shared<spi::impl::listener::listener_service_impl>(clientContext_, eventThreadCount);
            }

            std::shared_ptr<spi::impl::ClientExecutionServiceImpl>
            HazelcastClientInstanceImpl::init_execution_service() {
                return std::make_shared<spi::impl::ClientExecutionServiceImpl>(instanceName_, clientProperties_,
                                                                               clientConfig_.get_executor_pool_size(),
                                                                               lifecycleService_);
            }

            std::shared_ptr<connection::ClientConnectionManagerImpl>
            HazelcastClientInstanceImpl::init_connection_manager_service(
                    const std::vector<std::shared_ptr<connection::AddressProvider>> &address_providers) {
                config::ClientAwsConfig &awsConfig = clientConfig_.get_network_config().get_aws_config();
                std::shared_ptr<connection::AddressTranslator> addressTranslator;
                if (awsConfig.is_enabled()) {
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
                        clientContext_, addressTranslator, address_providers);

            }

            void HazelcastClientInstanceImpl::on_cluster_restart() {
                HZ_LOG(*logger_, info,
                    "Clearing local state of the client, because of a cluster restart");

                nearCacheManager_->clear_all_near_caches();
                //clear the member list version
                clusterService_.clear_member_list_version();
            }

            std::vector<std::shared_ptr<connection::AddressProvider>>
            HazelcastClientInstanceImpl::create_address_providers() {
                config::ClientNetworkConfig &networkConfig = get_client_config().get_network_config();
                config::ClientAwsConfig &awsConfig = networkConfig.get_aws_config();
                std::vector<std::shared_ptr<connection::AddressProvider>> addressProviders;

                if (awsConfig.is_enabled()) {
                    int awsMemberPort = clientProperties_.get_integer(clientProperties_.get_aws_member_port());
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

            const std::string &HazelcastClientInstanceImpl::get_name() const {
                return instanceName_;
            }

            spi::LifecycleService &HazelcastClientInstanceImpl::get_lifecycle_service() {
                return lifecycleService_;
            }

            const std::shared_ptr<ClientLockReferenceIdGenerator> &
            HazelcastClientInstanceImpl::get_lock_reference_id_generator() const {
                return lockReferenceIdGenerator_;
            }

            spi::ProxyManager &HazelcastClientInstanceImpl::get_proxy_manager() {
                return proxyManager_;
            }

            void HazelcastClientInstanceImpl::initalize_near_cache_manager() {
                nearCacheManager_.reset(
                        new internal::nearcache::NearCacheManager(executionService_, serializationService_, *logger_));
            }

            Client HazelcastClientInstanceImpl::get_local_endpoint() const {
                return clusterService_.get_local_client();
            }

            template<>
            std::shared_ptr<IMap> HazelcastClientInstanceImpl::get_distributed_object(const std::string& name) {
                auto nearCacheConfig = clientConfig_.get_near_cache_config(name);
                if (nearCacheConfig) {
                    return proxyManager_.get_or_create_proxy<map::NearCachedClientMapProxy<serialization::pimpl::Data, serialization::pimpl::Data>>(
                            IMap::SERVICE_NAME, name);
                } else {
                    return proxyManager_.get_or_create_proxy<IMap>(IMap::SERVICE_NAME, name);
                }
            }

            const std::shared_ptr<logger> &HazelcastClientInstanceImpl::get_logger() const {
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

            void BaseEventHandler::set_logger(logger *lg) {
                BaseEventHandler::logger_ = lg;
            }

            logger *BaseEventHandler::get_logger() const {
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

        Address::Address(const std::string &hostname, int port, unsigned long scope_id) : host_(hostname), port_(port),
                                                                                         type_(IPV6), scopeId_(scope_id) {
        }

        bool Address::operator==(const Address &rhs) const {
            return rhs.port_ == port_ && rhs.type_ == type_ && 0 == rhs.host_.compare(host_);
        }

        bool Address::operator!=(const Address &rhs) const {
            return !(*this == rhs);
        }

        int Address::get_port() const {
            return port_;
        }

        const std::string &Address::get_host() const {
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

        bool Address::is_ip_v4() const {
            return type_ == IPV4;
        }

        unsigned long Address::get_scope_id() const {
            return scopeId_;
        }

        std::string Address::to_string() const {
            std::ostringstream out;
            out << "Address[" << get_host() << ":" << get_port() << "]";
            return out.str();
        }

        std::ostream &operator<<(std::ostream &stream, const Address &address) {
            return stream << address.to_string();
        }

        namespace serialization {
            int32_t hz_serializer<Address>::get_factory_id() {
                return F_ID;
            }

            int32_t hz_serializer<Address>::get_class_id() {
                return ADDRESS;
            }

            void hz_serializer<Address>::write_data(const Address &object, ObjectDataOutput &out) {
                out.write<int32_t>(object.port_);
                out.write<byte>(object.type_);
                out.write(object.host_);
            }

            Address hz_serializer<Address>::read_data(ObjectDataInput &in) {
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
        IExecutorService::select_members(const cluster::memberselector::MemberSelector &member_selector) {
            std::vector<Member> selected;
            std::vector<Member> members = get_context().get_client_cluster_service().get_member_list();
            for (const Member &member : members) {
                if (member_selector.select(member)) {
                    selected.push_back(member);
                }
            }
            if (selected.empty()) {
                throw (exception::ExceptionBuilder<exception::RejectedExecutionException>(
                        "IExecutorService::selectMembers") << "No member selected with memberSelector["
                                                           << member_selector << "]").build();
            }
            return selected;
        }

        std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
        IExecutorService::invoke_on_target(protocol::ClientMessage &&request, boost::uuids::uuid target) {
            try {
                std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        get_context(), request, get_name(), target);
                return std::make_pair(clientInvocation->invoke(), clientInvocation);
            } catch (exception::IException &) {
                util::ExceptionUtil::rethrow(std::current_exception());
            }
            return std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>();
        }

        std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
        IExecutorService::invoke_on_partition_owner(protocol::ClientMessage &&request, int partition_id) {
            try {
                std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        get_context(), request, get_name(), partition_id);
                return std::make_pair(clientInvocation->invoke(), clientInvocation);
            } catch (exception::IException &) {
                util::ExceptionUtil::rethrow(std::current_exception());
            }
            return std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>();
        }

        bool IExecutorService::is_sync_computation(bool prevent_sync) {
            int64_t now = util::current_time_millis();

            int64_t last = lastSubmitTime_;
            lastSubmitTime_ = now;

            if (last + MIN_TIME_RESOLUTION_OF_CONSECUTIVE_SUBMITS < now) {
                consecutiveSubmits_ = 0;
                return false;
            }

            return !prevent_sync && (consecutiveSubmits_++ % MAX_CONSECUTIVE_SUBMITS == 0);
        }

        Address IExecutorService::get_member_address(const Member &member) {
            auto m = get_context().get_client_cluster_service().get_member(member.get_uuid());
            if (!m) {
                throw (exception::ExceptionBuilder<exception::HazelcastException>(
                        "IExecutorService::getMemberAddress(Member)") << member << " is not available!").build();
            }
            return m->get_address();
        }

        int IExecutorService::random_partition_id() {
            auto &partitionService = get_context().get_partition_service();
            return rand() % partitionService.get_partition_count();
        }

        void IExecutorService::shutdown() {
            auto request = protocol::codec::executorservice_shutdown_encode(
                    get_name());
            invoke(request);
        }

        boost::future<bool> IExecutorService::is_shutdown() {
            auto request = protocol::codec::executorservice_isshutdown_encode(
                    get_name());
            return invoke_and_get_future<bool>(
                    request);
        }

        boost::future<bool> IExecutorService::is_terminated() {
            return is_shutdown();
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

        ClientProperty::ClientProperty(const std::string &name, const std::string &default_value)
                : name_(name), defaultValue_(default_value) {
        }

        const std::string &ClientProperty::get_name() const {
            return name_;
        }

        const std::string &ClientProperty::get_default_value() const {
            return defaultValue_;
        }

        const char *ClientProperty::get_system_property() const {
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

        const ClientProperty &ClientProperties::get_heartbeat_timeout() const {
            return heartbeatTimeout_;
        }

        const ClientProperty &ClientProperties::get_heartbeat_interval() const {
            return heartbeatInterval_;
        }

        const ClientProperty &ClientProperties::get_aws_member_port() const {
            return awsMemberPort_;
        }

        const ClientProperty &ClientProperties::get_clean_resources_period_millis() const {
            return cleanResourcesPeriod_;
        }

        const ClientProperty &ClientProperties::get_invocation_retry_pause_millis() const {
            return invocationRetryPauseMillis_;
        }

        const ClientProperty &ClientProperties::get_invocation_timeout_seconds() const {
            return invocationTimeoutSeconds_;
        }

        const ClientProperty &ClientProperties::get_event_thread_count() const {
            return eventThreadCount_;
        }

        const ClientProperty &ClientProperties::get_internal_executor_pool_size() const {
            return internalExecutorPoolSize_;
        }

        const ClientProperty &ClientProperties::get_shuffle_member_list() const {
            return shuffleMemberList_;
        }

        const ClientProperty &ClientProperties::get_max_concurrent_invocations() const {
            return maxConcurrentInvocations_;
        }

        const ClientProperty &ClientProperties::get_backpressure_backoff_timeout_millis() const {
            return backpressureBackoffTimeoutMillis_;
        }

        const ClientProperty &ClientProperties::get_statistics_enabled() const {
            return statisticsEnabled_;
        }

        const ClientProperty &ClientProperties::get_statistics_period_seconds() const {
            return statisticsPeriodSeconds_;
        }

        const ClientProperty &ClientProperties::get_io_thread_count() const {
            return ioThreadCount_;
        }

        std::string ClientProperties::get_string(const ClientProperty &property) const {
            std::unordered_map<std::string, std::string>::const_iterator valueIt = propertiesMap_.find(property.get_name());
            if (valueIt != propertiesMap_.end()) {
                return valueIt->second;
            }

            const char *value = property.get_system_property();
            if (value != NULL) {
                return value;
            }

            return property.get_default_value();
        }

        bool ClientProperties::get_boolean(const ClientProperty &property) const {
            return util::IOUtil::to_value<bool>(get_string(property));
        }

        int32_t ClientProperties::get_integer(const ClientProperty &property) const {
            return util::IOUtil::to_value<int32_t>(get_string(property));
        }

        int64_t ClientProperties::get_long(const ClientProperty &property) const {
            return util::IOUtil::to_value<int64_t>(get_string(property));
        }

        const ClientProperty &ClientProperties::get_response_executor_thread_count() const {
            return responseExecutorThreadCount_;
        }

        const ClientProperty &ClientProperties::backup_timeout_millis() const {
            return backup_timeout_millis_;
        }

        const ClientProperty &ClientProperties::fail_on_indeterminate_state() const {
            return fail_on_indeterminate_state_;
        }

        namespace exception {
            IException::IException(const std::string &exception_name, const std::string &source,
                                   const std::string &message, const std::string &details, int32_t error_no,
                                   std::exception_ptr cause, bool is_runtime, bool retryable)
                    : src_(source), msg_(message), details_(details), errorCode_(error_no), cause_(cause),
                    runtimeException_(is_runtime), retryable_(retryable), report_((boost::format(
                            "%1% {%2%. Error code:%3%, Details:%4%.} at %5%.") % exception_name % message % error_no %
                                                    details % source).str()) {
            }

            IException::~IException() noexcept = default;

            char const *IException::what() const noexcept {
                return report_.c_str();
            }

            const std::string &IException::get_source() const {
                return src_;
            }

            const std::string &IException::get_message() const {
                return msg_;
            }

            std::ostream &operator<<(std::ostream &os, const IException &exception) {
                os << exception.what();
                return os;
            }

            const std::string &IException::get_details() const {
                return details_;
            }

            int32_t IException::get_error_code() const {
                return errorCode_;
            }

            bool IException::is_runtime_exception() const {
                return runtimeException_;
            }

            bool IException::is_retryable() const {
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

            RetryableHazelcastException::RetryableHazelcastException(const std::string &error_name, int32_t error_code,
                                                                     const std::string &source,
                                                                     const std::string &message,
                                                                     const std::string &details, std::exception_ptr cause,
                                                                     bool runtime, bool retryable) : HazelcastException(error_name,
                                                                                                          error_code,
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
        return std::hash<std::string>()(address.get_host()) + std::hash<int>()(address.get_port()) +
               std::hash<unsigned long>()(address.type_);
    }
}
