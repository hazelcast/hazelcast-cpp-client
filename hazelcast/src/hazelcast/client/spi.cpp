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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
// required due to issue at asio https://github.com/chriskohlhoff/asio/issues/431
#include <boost/asio/detail/win_iocp_io_context.hpp>
#endif

#include <utility>

#include <boost/range/algorithm/find_if.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <boost/functional/hash.hpp>

#include "hazelcast/client/HazelcastClient.h"
#include <hazelcast/client/protocol/codec/ErrorCodec.h>
#include <hazelcast/client/spi/impl/ListenerMessageCodec.h>
#include <hazelcast/client/spi/impl/ClientClusterServiceImpl.h>
#include <hazelcast/client/spi/impl/listener/cluster_view_listener.h>
#include <hazelcast/client/spi/impl/listener/listener_service_impl.h>
#include "hazelcast/client/cluster/memberselector/MemberSelectors.h"
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/MembershipEvent.h"
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/client/spi/ProxyManager.h"
#include "hazelcast/client/spi/ClientProxy.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/impl/ClientInvocationServiceImpl.h"
#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/spi/impl/DefaultAddressProvider.h"
#include "hazelcast/client/spi/impl/AwsAddressProvider.h"
#include "hazelcast/client/spi/impl/DefaultAddressTranslator.h"
#include "hazelcast/client/spi/impl/sequence/CallIdSequenceWithBackpressure.h"
#include "hazelcast/client/spi/impl/sequence/CallIdSequenceWithoutBackpressure.h"
#include "hazelcast/client/spi/impl/sequence/FailFastCallIdSequence.h"
#include "hazelcast/client/spi/impl/sequence/CallIdFactory.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/util/AddressHelper.h"
#include "hazelcast/util/HashUtil.h"
#include "hazelcast/util/concurrent/BackoffIdleStrategy.h"

namespace hazelcast {
    namespace client {
        const std::unordered_set<Member> &InitialMembershipEvent::get_members() const {
            return members_;
        }

        Cluster &InitialMembershipEvent::get_cluster() {
            return cluster_;
        }

        InitialMembershipEvent::InitialMembershipEvent(Cluster &cluster, std::unordered_set<Member> members) : cluster_(
                cluster), members_(std::move(members)) {
        }

        LifecycleEvent::LifecycleEvent(LifecycleState state)
                : state_(state) {
        }

        LifecycleEvent::LifecycleState LifecycleEvent::get_state() const {
            return state_;
        }

        namespace spi {
            ProxyManager::ProxyManager(ClientContext &context) : client_(context) {
            }

            void ProxyManager::init() {
            }

            void ProxyManager::destroy() {
                std::lock_guard<std::mutex> guard(lock_);
                for (auto &p : proxies_) {
                    try {
                        auto proxy = p.second.get();
                        p.second.get()->on_shutdown();
                    } catch (std::exception &se) {
                        auto &lg = client_.get_logger();
                        HZ_LOG(lg, finest,
                            boost::str(boost::format("Proxy was not created, "
                                                     "hence onShutdown can be called. Exception: %1%")
                                                     % se.what())
                        );
                    }
                }
                proxies_.clear();
            }

            void ProxyManager::initialize(const std::shared_ptr<ClientProxy> &clientProxy) {
                auto clientMessage = protocol::codec::client_createproxy_encode(clientProxy->get_name(),
                        clientProxy->get_service_name());
                spi::impl::ClientInvocation::create(client_, clientMessage, clientProxy->get_service_name())->invoke().get();
                clientProxy->on_initialize();
            }

            boost::future<void> ProxyManager::destroy_proxy(ClientProxy &proxy) {
                DefaultObjectNamespace objectNamespace(proxy.get_service_name(), proxy.get_name());
                std::shared_ptr<ClientProxy> registeredProxy;
                {
                    std::lock_guard<std::mutex> guard(lock_);
                    auto it = proxies_.find(objectNamespace);
                    registeredProxy = it == proxies_.end() ? nullptr : it->second.get();
                    if (it != proxies_.end()) {
                        proxies_.erase(it);
                    }
                }

                try {
                    if (registeredProxy) {
                        try {
                            proxy.destroy_locally();
                            return proxy.destroy_remotely();
                        } catch (exception::IException &) {
                            proxy.destroy_remotely();
                            throw;
                        }
                    }
                    if (&proxy != registeredProxy.get()) {
                        // The given proxy is stale and was already destroyed, but the caller
                        // may have allocated local resources in the context of this stale proxy
                        // instance after it was destroyed, so we have to cleanup it locally one
                        // more time to make sure there are no leaking local resources.
                        proxy.destroy_locally();
                    }
                } catch (...) {
                    if (&proxy != registeredProxy.get()) {
                        // The given proxy is stale and was already destroyed, but the caller
                        // may have allocated local resources in the context of this stale proxy
                        // instance after it was destroyed, so we have to cleanup it locally one
                        // more time to make sure there are no leaking local resources.
                        proxy.destroy_locally();
                    }
                    throw;
                }
                return boost::make_ready_future();
            }

            ClientContext::ClientContext(const client::HazelcastClient &hazelcastClient) : hazelcastClient_(
                    *hazelcastClient.clientImpl_) {
            }

            ClientContext::ClientContext(client::impl::HazelcastClientInstanceImpl &hazelcastClient)
                    : hazelcastClient_(hazelcastClient) {
            }

            serialization::pimpl::SerializationService &ClientContext::get_serialization_service() {
                return hazelcastClient_.serializationService_;
            }

            impl::ClientClusterServiceImpl & ClientContext::get_client_cluster_service() {
                return hazelcastClient_.clusterService_;
            }

            impl::ClientInvocationServiceImpl &ClientContext::get_invocation_service() {
                return *hazelcastClient_.invocationService_;
            }

            ClientConfig &ClientContext::get_client_config() {
                return hazelcastClient_.clientConfig_;
            }

            impl::ClientPartitionServiceImpl & ClientContext::get_partition_service() {
                return *hazelcastClient_.partitionService_;
            }

            LifecycleService &ClientContext::get_lifecycle_service() {
                return hazelcastClient_.lifecycleService_;
            }

            spi::impl::listener::listener_service_impl &ClientContext::get_client_listener_service() {
                return *hazelcastClient_.listenerService_;
            }

            connection::ClientConnectionManagerImpl &ClientContext::get_connection_manager() {
                return *hazelcastClient_.connectionManager_;
            }

            internal::nearcache::NearCacheManager &ClientContext::get_near_cache_manager() {
                return *hazelcastClient_.nearCacheManager_;
            }

            ClientProperties &ClientContext::get_client_properties() {
                return hazelcastClient_.clientProperties_;
            }

            Cluster &ClientContext::get_cluster() {
                return hazelcastClient_.cluster_;
            }

            std::shared_ptr<impl::sequence::CallIdSequence> &ClientContext::get_call_id_sequence() const {
                return hazelcastClient_.callIdSequence_;
            }

            const protocol::ClientExceptionFactory &ClientContext::get_client_exception_factory() const {
                return hazelcastClient_.get_exception_factory();
            }

            const std::string &ClientContext::get_name() const {
                return hazelcastClient_.get_name();
            }

            impl::ClientExecutionServiceImpl &ClientContext::get_client_execution_service() const {
                return *hazelcastClient_.executionService_;
            }

            const std::shared_ptr<client::impl::ClientLockReferenceIdGenerator> &
            ClientContext::get_lock_reference_id_generator() {
                return hazelcastClient_.get_lock_reference_id_generator();
            }

            std::shared_ptr<client::impl::HazelcastClientInstanceImpl>
            ClientContext::get_hazelcast_client_implementation() {
                return hazelcastClient_.shared_from_this();
            }

            spi::ProxyManager &ClientContext::get_proxy_manager() {
                return hazelcastClient_.get_proxy_manager();
            }

            logger &ClientContext::get_logger() {
                return *hazelcastClient_.logger_;
            }

            client::impl::statistics::Statistics &ClientContext::get_clientstatistics() {
                return *hazelcastClient_.statistics_;
            }

            spi::impl::listener::cluster_view_listener &ClientContext::get_cluster_view_listener() {
                return *hazelcastClient_.cluster_listener_;
            }

            boost::uuids::uuid ClientContext::random_uuid() {
                return hazelcastClient_.random_uuid();
            }

            cp::internal::session::proxy_session_manager &ClientContext::get_proxy_session_manager() {
                return hazelcastClient_.proxy_session_manager_;
            }

            LifecycleService::LifecycleService(ClientContext &clientContext,
                                               const std::vector<LifecycleListener> &listeners,
                                               LoadBalancer *const loadBalancer, Cluster &cluster) : 
                    clientContext_(clientContext), listeners_(), loadBalancer_(loadBalancer),
                    cluster_(cluster), shutdownCompletedLatch_(1) {
                for (const auto &listener: listeners) {
                    add_listener(LifecycleListener(listener));
                }
            }

            bool LifecycleService::start() {
                bool expected = false;
                if (!active_.compare_exchange_strong(expected, true)) {
                    return false;
                }

                fire_lifecycle_event(LifecycleEvent::STARTED);

                clientContext_.get_client_execution_service().start();

                clientContext_.get_client_listener_service().start();

                clientContext_.get_invocation_service().start();

                clientContext_.get_client_cluster_service().start();

                clientContext_.get_cluster_view_listener().start();

                if (!clientContext_.get_connection_manager().start()) {
                    return false;
                }

                auto &connectionStrategyConfig = clientContext_.get_client_config().get_connection_strategy_config();
                if (!connectionStrategyConfig.is_async_start()) {
                    // The client needs to open connections to all members before any services requiring internal listeners start
                    wait_for_initial_membership_event();
                    clientContext_.get_connection_manager().connect_to_all_cluster_members();
                }

                clientContext_.get_invocation_service().add_backup_listener();

                loadBalancer_->init(cluster_);

                clientContext_.get_clientstatistics().start();

                return true;
            }

            void LifecycleService::shutdown() {
                bool expected = true;
                if (!active_.compare_exchange_strong(expected, false)) {
                    shutdownCompletedLatch_.wait();
                    return;
                }
                try {
                    fire_lifecycle_event(LifecycleEvent::SHUTTING_DOWN);
                    clientContext_.get_proxy_session_manager().shutdown();
                    clientContext_.get_clientstatistics().shutdown();
                    clientContext_.get_proxy_manager().destroy();
                    clientContext_.get_connection_manager().shutdown();
                    clientContext_.get_client_cluster_service().shutdown();
                    clientContext_.get_invocation_service().shutdown();
                    clientContext_.get_client_listener_service().shutdown();
                    clientContext_.get_near_cache_manager().destroy_all_near_caches();
                    fire_lifecycle_event(LifecycleEvent::SHUTDOWN);
                    clientContext_.get_client_execution_service().shutdown();
                    clientContext_.get_serialization_service().dispose();
                    shutdownCompletedLatch_.count_down();
                } catch (std::exception &e) {
                    HZ_LOG(clientContext_.get_logger(), info,
                        boost::str(boost::format("An exception occured during LifecycleService shutdown. %1%")
                                                 % e.what())
                    );
                    shutdownCompletedLatch_.count_down();
                }
            }

            boost::uuids::uuid LifecycleService::add_listener(LifecycleListener &&lifecycleListener) {
                std::lock_guard<std::mutex> lg(listenerLock_);
                const auto id = uuid_generator_();
                listeners_.emplace(id, std::move(lifecycleListener));
                return id;
            }

            bool LifecycleService::remove_listener(const boost::uuids::uuid &registrationId) {
                std::lock_guard<std::mutex> guard(listenerLock_);
                return listeners_.erase(registrationId) == 1;
            }

            void LifecycleService::fire_lifecycle_event(const LifecycleEvent &lifecycleEvent) {
                std::lock_guard<std::mutex> guard(listenerLock_);
                logger &lg = clientContext_.get_logger();

                std::function<void(LifecycleListener &)> fire_one;

                switch (lifecycleEvent.get_state()) {
                    case LifecycleEvent::STARTING : {
                        // convert the date string from "2016-04-20" to 20160420
                        std::string date(HAZELCAST_STRINGIZE(HAZELCAST_GIT_COMMIT_DATE));
                        util::git_date_to_hazelcast_log_date(date);
                        std::string commitId(HAZELCAST_STRINGIZE(HAZELCAST_GIT_COMMIT_ID));
                        commitId.erase(std::remove(commitId.begin(), commitId.end(), '"'), commitId.end());
                        char msg[100];
                        util::hz_snprintf(msg, 100, "(%s:%s) LifecycleService::LifecycleEvent STARTING", date.c_str(),
                                          commitId.c_str());
                        HZ_LOG(lg, info, msg);

                        fire_one = [](LifecycleListener &listener) {
                            listener.starting_();
                        };
                        break;
                    }
                    case LifecycleEvent::STARTED : {
                        HZ_LOG(lg, info, "LifecycleService::LifecycleEvent STARTED");

                        fire_one = [](LifecycleListener &listener) {
                            listener.started_();
                        };
                        break;
                    }
                    case LifecycleEvent::SHUTTING_DOWN : {
                        HZ_LOG(lg, info, "LifecycleService::LifecycleEvent SHUTTING_DOWN");

                        fire_one = [](LifecycleListener &listener) {
                            listener.shutting_down_();
                        };
                        break;
                    }
                    case LifecycleEvent::SHUTDOWN : {
                        HZ_LOG(lg, info, "LifecycleService::LifecycleEvent SHUTDOWN");

                        fire_one = [](LifecycleListener &listener) {
                            listener.shutdown_();
                        };
                        break;
                    }
                    case LifecycleEvent::CLIENT_CONNECTED : {
                        HZ_LOG(lg, info, "LifecycleService::LifecycleEvent CLIENT_CONNECTED");

                        fire_one = [](LifecycleListener &listener) {
                            listener.connected_();
                        };
                        break;
                    }
                    case LifecycleEvent::CLIENT_DISCONNECTED : {
                        HZ_LOG(lg, info, "LifecycleService::LifecycleEvent CLIENT_DISCONNECTED");

                        fire_one = [](LifecycleListener &listener) {
                            listener.disconnected_();
                        };
                        break;
                    }
                }

                for (auto &item: listeners_) {
                    fire_one(item.second);
                }
            }

            bool LifecycleService::is_running() {
                return active_;
            }

            LifecycleService::~LifecycleService() {
                if (active_) {
                    shutdown();
                }
            }

            void LifecycleService::wait_for_initial_membership_event() const {
                clientContext_.get_client_cluster_service().wait_initial_member_list_fetched();
            }

            DefaultObjectNamespace::DefaultObjectNamespace(const std::string &service, const std::string &object)
                    : serviceName_(service), objectName_(object) {

            }

            const std::string &DefaultObjectNamespace::get_service_name() const {
                return serviceName_;
            }

            const std::string &DefaultObjectNamespace::get_object_name() const {
                return objectName_;
            }

            bool DefaultObjectNamespace::operator==(const DefaultObjectNamespace &rhs) const {
                return serviceName_ == rhs.serviceName_ && objectName_ == rhs.objectName_;
            }

            ClientProxy::ClientProxy(const std::string &name, const std::string &serviceName, ClientContext &context)
                    : name_(name), serviceName_(serviceName), context_(context) {}

            ClientProxy::~ClientProxy() = default;

            const std::string &ClientProxy::get_name() const {
                return name_;
            }

            const std::string &ClientProxy::get_service_name() const {
                return serviceName_;
            }

            ClientContext &ClientProxy::get_context() {
                return context_;
            }

            void ClientProxy::on_destroy() {
            }

            boost::future<void> ClientProxy::destroy() {
                return get_context().get_proxy_manager().destroy_proxy(*this);
            }

            void ClientProxy::destroy_locally() {
                if (pre_destroy()) {
                    try {
                        on_destroy();
                        post_destroy();
                    } catch (exception::IException &) {
                        post_destroy();
                        throw;
                    }
                }
            }

            bool ClientProxy::pre_destroy() {
                return true;
            }

            void ClientProxy::post_destroy() {
            }

            void ClientProxy::on_initialize() {
            }

            void ClientProxy::on_shutdown() {
            }

            serialization::pimpl::SerializationService &ClientProxy::get_serialization_service() {
                return context_.get_serialization_service();
            }

            boost::future<void> ClientProxy::destroy_remotely() {
                auto clientMessage = protocol::codec::client_destroyproxy_encode(
                        get_name(), get_service_name());
                return spi::impl::ClientInvocation::create(get_context(), std::make_shared<protocol::ClientMessage>(std::move(clientMessage)), get_name())->invoke().then(
                        boost::launch::deferred, [](boost::future<protocol::ClientMessage> f) { f.get(); });
            }

            boost::future<boost::uuids::uuid>
            ClientProxy::register_listener(std::shared_ptr<impl::ListenerMessageCodec> listenerMessageCodec,
                                          std::shared_ptr<client::impl::BaseEventHandler> handler) {
                handler->set_logger(&get_context().get_logger());
                return get_context().get_client_listener_service().register_listener(listenerMessageCodec,
                                                                                handler);
            }

            boost::future<bool> ClientProxy::deregister_listener(boost::uuids::uuid registrationId) {
                return get_context().get_client_listener_service().deregister_listener(registrationId);
            }

            namespace impl {
                boost::uuids::uuid
                ListenerMessageCodec::decode_add_response(protocol::ClientMessage &msg) const {
                    return msg.get_first_uuid();
                }

                bool ListenerMessageCodec::decode_remove_response(protocol::ClientMessage &msg) const {
                    return msg.get_first_fixed_sized_field<bool>();
                }

                ClientInvocationServiceImpl::ClientInvocationServiceImpl(ClientContext &client)
                        : client_(client), logger_(client.get_logger()),
                          invocationTimeout_(std::chrono::seconds(client.get_client_properties().get_integer(
                                  client.get_client_properties().get_invocation_timeout_seconds()))),
                          invocationRetryPause_(std::chrono::milliseconds(client.get_client_properties().get_long(
                                  client.get_client_properties().get_invocation_retry_pause_millis()))),
                          responseThread_(logger_, *this, client),
                          smart_routing_(client.get_client_config().get_network_config().is_smart_routing()),
                          backup_acks_enabled_(smart_routing_ && client.get_client_config().backup_acks_enabled()),
                          fail_on_indeterminate_operation_state_(client.get_client_properties().get_boolean(client.get_client_properties().fail_on_indeterminate_state())),
                          backup_timeout_(std::chrono::milliseconds(client.get_client_properties().get_integer(client.get_client_properties().backup_timeout_millis()))) {}

                void ClientInvocationServiceImpl::start() {
                    responseThread_.start();
                }

                void ClientInvocationServiceImpl::add_backup_listener() {
                    if (this->backup_acks_enabled_) {
                        auto &listener_service = this->client_.get_client_listener_service();
                        listener_service.register_listener(std::make_shared<BackupListenerMessageCodec>(),
                                                          std::make_shared<noop_backup_event_handler>()).get();
                    }
                }

                void ClientInvocationServiceImpl::shutdown() {
                    isShutdown_.store(true);

                    responseThread_.shutdown();
                }

                std::chrono::milliseconds ClientInvocationServiceImpl::get_invocation_timeout() const {
                    return invocationTimeout_;
                }

                std::chrono::milliseconds ClientInvocationServiceImpl::get_invocation_retry_pause() const {
                    return invocationRetryPause_;
                }

                bool ClientInvocationServiceImpl::is_redo_operation() {
                    return client_.get_client_config().is_redo_operation();
                }

                void
                ClientInvocationServiceImpl::handle_client_message(const std::shared_ptr<ClientInvocation> &invocation,
                                                                 const std::shared_ptr<ClientMessage> &response) {
                    responseThread_.process(invocation, response);
                }

                bool ClientInvocationServiceImpl::send(const std::shared_ptr<impl::ClientInvocation>& invocation,
                                                           const std::shared_ptr<connection::Connection>& connection) {
                    if (isShutdown_) {
                        BOOST_THROW_EXCEPTION(
                                exception::HazelcastClientNotActiveException("ClientInvocationServiceImpl::send",
                                                                             "Client is shut down"));
                    }

                    if (backup_acks_enabled_) {
                        invocation->get_client_message()->add_flag(protocol::ClientMessage::BACKUP_AWARE_FLAG);
                    }

                    write_to_connection(*connection, invocation);
                    invocation->set_send_connection(connection);
                    return true;
                }

                void ClientInvocationServiceImpl::write_to_connection(connection::Connection &connection,
                                                                        const std::shared_ptr<ClientInvocation> &clientInvocation) {
                    auto clientMessage = clientInvocation->get_client_message();
                    connection.write(clientInvocation);
                }

                void ClientInvocationServiceImpl::check_invocation_allowed() {
                    client_.get_connection_manager().check_invocation_allowed();
                }

                bool ClientInvocationServiceImpl::invoke(std::shared_ptr<ClientInvocation> invocation) {
                    auto connection = client_.get_connection_manager().get_random_connection();
                    if (!connection) {
                        HZ_LOG(logger_, finest, "No connection found to invoke");
                        return false;
                    }
                    return send(invocation, connection);
                }

                ClientInvocationServiceImpl::ResponseProcessor::ResponseProcessor(logger &lg,
                                                                                  ClientInvocationServiceImpl &invocationService,
                                                                                  ClientContext &clientContext)
                        : logger_(lg), client_(clientContext) {
                }

                void ClientInvocationServiceImpl::ResponseProcessor::process_internal(
                        const std::shared_ptr<ClientInvocation> &invocation,
                        const std::shared_ptr<protocol::ClientMessage> &response) {
                    try {
                        if (protocol::codec::ErrorCodec::EXCEPTION_MESSAGE_TYPE == response->get_message_type()) {
                            auto error_holder = protocol::codec::ErrorCodec::decode(*response);
                            invocation->notify_exception(client_.get_client_exception_factory().create_exception(error_holder));
                        } else {
                            invocation->notify(response);
                        }
                    } catch (std::exception &e) {
                        HZ_LOG(logger_, severe, 
                            boost::str(boost::format("Failed to process response for %1%. %2%")
                                                     % *invocation % e.what())
                        );
                    }
                }

                void ClientInvocationServiceImpl::ResponseProcessor::shutdown() {
                    ClientExecutionServiceImpl::shutdown_thread_pool(pool_.get());
                }

                void ClientInvocationServiceImpl::ResponseProcessor::start() {
                    ClientProperties &clientProperties = client_.get_client_properties();
                    auto threadCount = clientProperties.get_integer(clientProperties.get_response_executor_thread_count());
                    if (threadCount > 0) {
                        pool_.reset(new hazelcast::util::hz_thread_pool(threadCount));
                    }
                }

                ClientInvocationServiceImpl::ResponseProcessor::~ResponseProcessor() {
                    shutdown();
                }

                void ClientInvocationServiceImpl::ResponseProcessor::process(
                        const std::shared_ptr<ClientInvocation> &invocation,
                        const std::shared_ptr<ClientMessage> &response) {
                    if (!pool_) {
                        process_internal(invocation, response);
                        return;
                    }

                    boost::asio::post(pool_->get_executor(), [=] { process_internal(invocation, response); });
                }

                DefaultAddressProvider::DefaultAddressProvider(config::ClientNetworkConfig &networkConfig,
                                                               bool noOtherAddressProviderExist) : networkConfig_(
                        networkConfig), noOtherAddressProviderExist_(noOtherAddressProviderExist) {
                }

                std::vector<Address> DefaultAddressProvider::load_addresses() {
                    std::vector<Address> addresses = networkConfig_.get_addresses();
                    if (addresses.empty() && noOtherAddressProviderExist_) {
                        addresses.emplace_back("127.0.0.1", 5701);
                    }

                    // TODO Implement AddressHelper to add alternative ports for the same address

                    return addresses;
                }

                const boost::shared_ptr<ClientClusterServiceImpl::member_list_snapshot> ClientClusterServiceImpl::EMPTY_SNAPSHOT(
                        new ClientClusterServiceImpl::member_list_snapshot{-1});

                constexpr boost::chrono::milliseconds ClientClusterServiceImpl::INITIAL_MEMBERS_TIMEOUT;

                ClientClusterServiceImpl::ClientClusterServiceImpl(hazelcast::client::spi::ClientContext &client)
                        : client_(client), member_list_snapshot_(EMPTY_SNAPSHOT), labels_(client.get_client_config().get_labels()),
                        initial_list_fetched_latch_(1) {
                }

                boost::uuids::uuid ClientClusterServiceImpl::add_membership_listener_without_init(
                        MembershipListener &&listener) {
                    std::lock_guard<std::mutex> g(listeners_lock_);
                    auto id = client_.random_uuid();
                    listeners_.emplace(id, std::move(listener));
                    return id;
                }

                boost::optional<Member> ClientClusterServiceImpl::get_member(boost::uuids::uuid uuid) const {
                    assert(!uuid.is_nil());
                    auto members_view_ptr = member_list_snapshot_.load();
                    const auto it = members_view_ptr->members.find(uuid);
                    if (it == members_view_ptr->members.end()) {
                        return boost::none;
                    }
                    return {it->second};
                }

                std::vector<Member> ClientClusterServiceImpl::get_member_list() const {
                    auto members_view_ptr = member_list_snapshot_.load();
                    std::vector<Member> result;
                    result.reserve(members_view_ptr->members.size());
                    for (const auto &e : members_view_ptr->members) {
                        result.emplace_back(e.second);
                    }
                    return result;
                }

                void ClientClusterServiceImpl::start() {
                    for (auto &listener : client_.get_client_config().get_membership_listeners()) {
                        add_membership_listener(MembershipListener(listener));
                    }
                }

                void ClientClusterServiceImpl::fire_initial_membership_event(const InitialMembershipEvent &event) {
                    std::lock_guard<std::mutex> g(listeners_lock_);

                    for (auto &item : listeners_) {
                        MembershipListener &listener = item.second;
                        if (listener.init_) {
                            listener.init_(event);
                        }
                    }
                }

                void ClientClusterServiceImpl::shutdown() {
                    initial_list_fetched_latch_.try_count_down();
                }

                boost::uuids::uuid
                ClientClusterServiceImpl::add_membership_listener(MembershipListener &&listener) {
                    std::lock_guard<std::mutex> cluster_view_g(cluster_view_lock_);

                    auto id = add_membership_listener_without_init(std::move(listener));

                    std::lock_guard<std::mutex> listeners_g(listeners_lock_);
                    auto added_listener = listeners_[id];

                    if (added_listener.init_) {
                        auto &cluster = client_.get_cluster();
                        auto members_ptr = member_list_snapshot_.load();
                        if (!members_ptr->members.empty()) {
                            std::unordered_set<Member> members;
                            for (const auto &e : members_ptr->members) {
                                members.insert(e.second);
                            }
                            added_listener.init_(InitialMembershipEvent(cluster, members));
                        }
                    }

                    return id;
                }

                bool ClientClusterServiceImpl::remove_membership_listener(boost::uuids::uuid registrationId) {
                    std::lock_guard<std::mutex> g(listeners_lock_);
                    return listeners_.erase(registrationId) == 1;
                }

                std::vector<Member>
                ClientClusterServiceImpl::get_members(const cluster::memberselector::MemberSelector &selector) const {
                    std::vector<Member> result;
                    for (auto &&member : get_member_list()) {
                        if (selector.select(member)) {
                            result.emplace_back(std::move(member));
                        }
                    }

                    return result;
                }

                Client ClientClusterServiceImpl::get_local_client() const {
                    connection::ClientConnectionManagerImpl &cm = client_.get_connection_manager();
                    auto connection = cm.get_random_connection();
                    auto inetSocketAddress = connection ? connection->get_local_socket_address() : boost::none;
                    auto uuid = cm.get_client_uuid();
                    return Client(uuid, std::move(inetSocketAddress), client_.get_name(), labels_);
                }

                void ClientClusterServiceImpl::clear_member_list_version() {
                    std::lock_guard<std::mutex> g(cluster_view_lock_);
                    auto &lg = client_.get_logger();
                    HZ_LOG(lg, finest, "Resetting the member list version ");
                    auto cluster_view_snapshot = member_list_snapshot_.load();
                    //This check is necessary so that `clear_member_list_version` when handling auth response will not
                    //intervene with client failover logic
                    if (cluster_view_snapshot != EMPTY_SNAPSHOT) {
                        member_list_snapshot_.store(boost::shared_ptr<member_list_snapshot>(
                                new member_list_snapshot{0, cluster_view_snapshot->members}));
                    }
                }

                void
                ClientClusterServiceImpl::handle_event(int32_t version, const std::vector<Member> &memberInfos) {
                    auto &lg = client_.get_logger();
                    HZ_LOG(lg, finest, 
                        boost::str(boost::format("Handling new snapshot with membership version: %1%, "
                                                 "membersString %2%")
                                                 % version
                                                 % members_string(create_snapshot(version, memberInfos)))
                    );
                    auto cluster_view_snapshot = member_list_snapshot_.load();
                    if (cluster_view_snapshot == EMPTY_SNAPSHOT) {
                        std::lock_guard<std::mutex> g(cluster_view_lock_);
                        cluster_view_snapshot = member_list_snapshot_.load();
                        if (cluster_view_snapshot == EMPTY_SNAPSHOT) {
                            //this means this is the first time client connected to cluster
                            apply_initial_state(version, memberInfos);
                            initial_list_fetched_latch_.count_down();
                            return;
                        }
                    }

                    std::vector<MembershipEvent> events;
                    if (version >= cluster_view_snapshot->version) {
                        std::lock_guard<std::mutex> g(cluster_view_lock_);
                        cluster_view_snapshot = member_list_snapshot_.load();
                        if (version >= cluster_view_snapshot->version) {
                            auto prev_members = cluster_view_snapshot->members;
                            auto snapshot = boost::make_shared<member_list_snapshot>(create_snapshot(version, memberInfos));
                            member_list_snapshot_.store(snapshot);
                            events = detect_membership_events(prev_members, snapshot->members);
                        }
                    }

                    fire_events(std::move(events));
                }

                ClientClusterServiceImpl::member_list_snapshot
                ClientClusterServiceImpl::create_snapshot(int32_t version, const std::vector<Member> &members) {
                    member_list_snapshot result;
                    result.version = version;
                    for (auto &m : members) {
                        result.members.insert({m.get_uuid(), m});
                    }

                    return result;
                }

                std::string
                ClientClusterServiceImpl::members_string(const ClientClusterServiceImpl::member_list_snapshot& snapshot) {
                    std::stringstream out;
                    auto const &members = snapshot.members;
                    out << std::endl << std::endl << "Members [" << members.size() << "]  {";
                    for (auto const &e : members) {
                        out << std::endl << "\t" << e.second;
                    }
                    out << std::endl << "}" << std::endl;
                    return out.str();
                }

                void
                ClientClusterServiceImpl::apply_initial_state(int32_t version, const std::vector<Member> &memberInfos) {
                    auto snapshot = boost::make_shared<member_list_snapshot>(create_snapshot(version, memberInfos));
                    member_list_snapshot_.store(snapshot);
                    HZ_LOG(client_.get_logger(), info, members_string(*snapshot));
                    std::unordered_set<Member> members;
                    for(auto const &e : snapshot->members) {
                        members.insert(e.second);
                    }
                    std::lock_guard<std::mutex> g(listeners_lock_);
                    for (auto &item : listeners_) {
                        MembershipListener &listener = item.second;
                        if (listener.init_) {
                            listener.init_(InitialMembershipEvent(client_.get_cluster(), members));
                        }
                    }
                }

                std::vector<MembershipEvent> ClientClusterServiceImpl::detect_membership_events(
                        std::unordered_map<boost::uuids::uuid, Member, boost::hash<boost::uuids::uuid>> previous_members,
                        const std::unordered_map<boost::uuids::uuid, Member, boost::hash<boost::uuids::uuid>>& current_members) {
                    std::vector<Member> new_members;

                    for (auto const & e : current_members) {
                        if (!previous_members.erase(e.first)) {
                            new_members.emplace_back(e.second);
                        }
                    }

                    std::vector<MembershipEvent> events;

                    // removal events should be added before added events
                    for (auto const &e : previous_members) {
                        events.emplace_back(client_.get_cluster(), e.second, MembershipEvent::MembershipEventType::MEMBER_LEFT, current_members);
                        auto connection = client_.get_connection_manager().get_connection(e.second.get_uuid());
                        if (connection) {
                            connection->close("", std::make_exception_ptr(exception::TargetDisconnectedException(
                                    "ClientClusterServiceImpl::detect_membership_events", (boost::format(
                                            "The client has closed the connection to this member, after receiving a member left event from the cluster. %1%") %
                                                                                           *connection).str())));
                        }
                    }
                    for (auto const &member : new_members) {
                        events.emplace_back(client_.get_cluster(), member, MembershipEvent::MembershipEventType::MEMBER_JOINED, current_members);
                    }

                    if (!events.empty()) {
                        auto snapshot = member_list_snapshot_.load();
                        if (!snapshot->members.empty()) {
                            HZ_LOG(client_.get_logger(), info, members_string(*snapshot));
                        }
                    }
                    return events;
                }

                void ClientClusterServiceImpl::fire_events(std::vector<MembershipEvent> events) {
                    std::lock_guard<std::mutex> g(listeners_lock_);
                    
                    for (auto const &event : events) {
                        for (auto &item : listeners_) {
                            MembershipListener &listener = item.second;
                            if (event.get_event_type() == MembershipEvent::MembershipEventType::MEMBER_JOINED) {
                                listener.joined_(event);
                            } else {
                                listener.left_(event);
                            }
                        }
                    }
                }

                void ClientClusterServiceImpl::wait_initial_member_list_fetched() const {
                    // safe to const cast here since latch operations are already thread safe ops.
                    if ((const_cast<boost::latch&>(initial_list_fetched_latch_)).wait_for(INITIAL_MEMBERS_TIMEOUT) == boost::cv_status::timeout) {
                        BOOST_THROW_EXCEPTION(exception::IllegalStateException(
                                                      "ClientClusterServiceImpl::wait_initial_member_list_fetched",
                                                              "Could not get initial member list from cluster!"));
                    }
                }

                bool
                ClientInvocationServiceImpl::invoke_on_connection(const std::shared_ptr<ClientInvocation> &invocation,
                                                                const std::shared_ptr<connection::Connection> &connection) {
                    return send(invocation, connection);
                }

                bool ClientInvocationServiceImpl::invoke_on_partition_owner(
                        const std::shared_ptr<ClientInvocation> &invocation, int partitionId) {
                    auto partition_owner = client_.get_partition_service().get_partition_owner(partitionId);
                    if (partition_owner.is_nil()) {
                        HZ_LOG(logger_, finest, 
                            boost::str(boost::format("Partition owner is not assigned yet for partition %1%")
                                                     % partitionId)
                        );
                        return false;
                    }
                    return invoke_on_target(invocation, partition_owner);
                }

                bool ClientInvocationServiceImpl::invoke_on_target(const std::shared_ptr<ClientInvocation> &invocation,
                                                                 boost::uuids::uuid uuid) {
                    assert (!uuid.is_nil());
                    auto connection = client_.get_connection_manager().get_connection(uuid);
                    if (!connection) {
                        HZ_LOG(logger_, finest, 
                            boost::str(boost::format("Client is not connected to target : %1%")
                                                     % uuid)
                        );
                        return false;
                    }
                    return send(invocation, connection);
                }

                bool ClientInvocationServiceImpl::is_smart_routing() const {
                    return smart_routing_;
                }

                const std::chrono::milliseconds &ClientInvocationServiceImpl::get_backup_timeout() const {
                    return backup_timeout_;
                }

                bool ClientInvocationServiceImpl::fail_on_indeterminate_state() const {
                    return fail_on_indeterminate_operation_state_;
                }

                ClientExecutionServiceImpl::ClientExecutionServiceImpl(const std::string &name,
                                                                       const ClientProperties &properties,
                                                                       int32_t poolSize,
                                                                       spi::LifecycleService &service)
                        : lifecycleService_(service), clientProperties_(properties), userExecutorPoolSize_(poolSize) {}

                void ClientExecutionServiceImpl::start() {
                    int internalPoolSize = clientProperties_.get_integer(clientProperties_.get_internal_executor_pool_size());
                    if (internalPoolSize <= 0) {
                        internalPoolSize = util::IOUtil::to_value<int>(
                                ClientProperties::INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT);
                    }

                    if (userExecutorPoolSize_ <= 0) {
                        userExecutorPoolSize_ = std::thread::hardware_concurrency();
                    }
                    if (userExecutorPoolSize_ <= 0) {
                        userExecutorPoolSize_ = 4; // hard coded thread pool count in case we could not get the processor count
                    }

                    internalExecutor_.reset(new hazelcast::util::hz_thread_pool(internalPoolSize));
                    userExecutor_.reset(new hazelcast::util::hz_thread_pool(userExecutorPoolSize_));
                }

                void ClientExecutionServiceImpl::shutdown() {
                    shutdown_thread_pool(userExecutor_.get());
                    shutdown_thread_pool(internalExecutor_.get());
                }

                boost::asio::thread_pool::executor_type ClientExecutionServiceImpl::get_user_executor() const {
                    return userExecutor_->get_executor();
                }

                void ClientExecutionServiceImpl::shutdown_thread_pool(hazelcast::util::hz_thread_pool *pool) {
                    if (!pool) {
                        return;
                    }
                    pool->shutdown_gracefully();
                }

                constexpr int ClientInvocation::MAX_FAST_INVOCATION_COUNT;
                constexpr int ClientInvocation::UNASSIGNED_PARTITION;

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   std::shared_ptr<protocol::ClientMessage> &&message,
                                                   const std::string &name,
                                                   int partition,
                                                   const std::shared_ptr<connection::Connection> &conn,
                                                   boost::uuids::uuid uuid) :
                        logger_(clientContext.get_logger()),
                        lifecycleService_(clientContext.get_lifecycle_service()),
                        clientClusterService_(clientContext.get_client_cluster_service()),
                        invocationService_(clientContext.get_invocation_service()),
                        executionService_(clientContext.get_client_execution_service().shared_from_this()),
                        callIdSequence_(clientContext.get_call_id_sequence()),
                        uuid_(uuid),
                        partitionId_(partition),
                        startTime_(std::chrono::steady_clock::now()),
                        retryPause_(invocationService_.get_invocation_retry_pause()),
                        objectName_(name),
                        connection_(conn),
                        invokeCount_(0), urgent_(false), smart_routing_(invocationService_.is_smart_routing()) {
                    message->set_partition_id(partitionId_);
                    clientMessage_ = boost::make_shared<std::shared_ptr<protocol::ClientMessage>>(message);
                    set_send_connection(nullptr);
                }

                ClientInvocation::~ClientInvocation() = default;

                boost::future<protocol::ClientMessage> ClientInvocation::invoke() {
                    assert (clientMessage_.load());
                    // for back pressure
                    callIdSequence_->next();
                    invoke_on_selection();
                    return invocationPromise_.get_future().then(boost::launch::sync,
                                                               [=](boost::future<protocol::ClientMessage> f) {
                                                                   callIdSequence_->complete();
                                                                   return f.get();
                                                               });
                }

                boost::future<protocol::ClientMessage> ClientInvocation::invoke_urgent() {
                    assert(clientMessage_.load());
                    urgent_ = true;
                    // for back pressure
                    callIdSequence_->force_next();
                    invoke_on_selection();
                    return invocationPromise_.get_future().then(boost::launch::sync,
                                                               [=](boost::future<protocol::ClientMessage> f) {
                                                                   callIdSequence_->complete();
                                                                   return f.get();
                                                               });
                }

                void ClientInvocation::invoke_on_selection() {
                    try {
                        invokeCount_++;
                        if (!urgent_) {
                            invocationService_.check_invocation_allowed();
                        }

                        if (is_bind_to_single_connection()) {
                            auto invoked = invocationService_.invoke_on_connection(shared_from_this(), connection_);
                            if (!invoked) {
                                notify_exception(std::make_exception_ptr(exception::IOException("", (boost::format(
                                        "Could not invoke on connection %1%") % *connection_).str())));
                            }
                            return;
                        }

                        bool invoked = false;
                        if (smart_routing_) {
                            if (partitionId_ != -1) {
                                invoked = invocationService_.invoke_on_partition_owner(shared_from_this(), partitionId_);
                            } else if (!uuid_.is_nil()) {
                                invoked = invocationService_.invoke_on_target(shared_from_this(), uuid_);
                            } else {
                                invoked = invocationService_.invoke(shared_from_this());
                            }
                            if (!invoked) {
                                invoked = invocationService_.invoke(shared_from_this());
                            }
                        } else {
                            invoked = invocationService_.invoke(shared_from_this());
                        }
                        if (!invoked) {
                            notify_exception(std::make_exception_ptr(exception::IOException("No connection found to invoke")));
                        }
                    } catch (exception::IException &) {
                        notify_exception(std::current_exception());
                    } catch (std::exception &) {
                        assert(false);
                    }
                }

                bool ClientInvocation::is_bind_to_single_connection() const {
                    return connection_ != nullptr;
                }

                void ClientInvocation::run() {
                    retry();
                }

                void ClientInvocation::retry() {
                    // retry modifies the client message and should not reuse the client message.
                    // It could be the case that it is in write queue of the connection.
                    clientMessage_ = boost::make_shared<std::shared_ptr<protocol::ClientMessage>>(copy_message());

                    try {
                        invoke_on_selection();
                    } catch (exception::IException &e) {
                        set_exception(e, boost::current_exception());
                    } catch (std::exception &) {
                        assert(false);
                    }
                }

                void ClientInvocation::set_exception(const exception::IException &e, boost::exception_ptr exceptionPtr) {
                    try {
                        auto send_conn = *sendConnection_.load();
                        if (send_conn) {
                            auto call_id = clientMessage_.load()->get()->get_correlation_id();
                            boost::asio::post(send_conn->get_socket().get_executor(), [=] () {
                                send_conn->deregister_invocation(call_id);
                            });
                        }
                        invocationPromise_.set_exception(std::move(exceptionPtr));
                    } catch (boost::promise_already_satisfied &se) {
                        if (!eventHandler_) {
                            HZ_LOG(logger_, finest,
                                boost::str(boost::format("Failed to set the exception for invocation. "
                                                         "%1%, %2% Exception to be set: %3%")
                                                         % se.what() % *this % e)
                            );
                        }
                    }
                }

                void ClientInvocation::notify_exception(std::exception_ptr exception) {
                    erase_invocation();
                    try {
                        std::rethrow_exception(exception);
                    } catch (exception::IException &iex) {
                        log_exception(iex);

                        if (!lifecycleService_.is_running()) {
                            try {
                                std::throw_with_nested(boost::enable_current_exception(
                                        exception::HazelcastClientNotActiveException(iex.get_source(),
                                                                                     "Client is shutting down")));
                            } catch (exception::IException &e) {
                                set_exception(e, boost::current_exception());
                            }
                            return;
                        }

                        if (!should_retry(iex)) {
                            set_exception(iex, boost::current_exception());
                            return;
                        }

                        auto timePassed = std::chrono::steady_clock::now() - startTime_;
                        if (timePassed > invocationService_.get_invocation_timeout()) {
                            HZ_LOG(logger_, finest,
                                boost::str(boost::format("Exception will not be retried because "
                                                         "invocation timed out. %1%") % iex.what())
                            );

                            auto now = std::chrono::steady_clock::now();

                            auto timeoutException = (exception::ExceptionBuilder<exception::OperationTimeoutException>(
                                    "ClientInvocation::newOperationTimeoutException") << *this
                                            << " timed out because exception occurred after client invocation timeout "
                                            << std::chrono::duration_cast<std::chrono::milliseconds>(invocationService_.get_invocation_timeout()).count()
                                            << "msecs. Last exception:" << iex
                                            << " Current time :" << util::StringUtil::time_to_string(now) << ". "
                                            << "Start time: " << util::StringUtil::time_to_string(startTime_)
                                            << ". Total elapsed time: " <<
                                            std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_).count()
                                            << " ms. ").build();
                            try {
                                BOOST_THROW_EXCEPTION(timeoutException);
                            } catch (...) {
                                set_exception(timeoutException, boost::current_exception());
                            }

                            return;
                        }

                        try {
                            execute();
                        } catch (exception::IException &e) {
                            set_exception(e, boost::current_exception());
                        }
                    } catch (...) {
                        assert(false);
                    }
                }

                void ClientInvocation::erase_invocation() const {
                    if (!this->eventHandler_) {
                        auto sent_connection = get_send_connection();
                        if (sent_connection) {
                            auto this_invocation = shared_from_this();
                            boost::asio::post(sent_connection->get_socket().get_executor(), [=] () {
                                sent_connection->invocations.erase(this_invocation->get_client_message()->get_correlation_id());
                            });
                        }
                    }
                }

                bool ClientInvocation::should_retry(exception::IException &exception) {
                    auto errorCode = exception.get_error_code();
                    if (is_bind_to_single_connection() && (errorCode == protocol::IO || errorCode == protocol::TARGET_DISCONNECTED)) {
                        return false;
                    }

                    if (!uuid_.is_nil() && errorCode == protocol::TARGET_NOT_MEMBER) {
                        //when invocation send to a specific member
                        //if target is no longer a member, we should not retry
                        //note that this exception could come from the server
                        return false;
                    }

                    if (errorCode == protocol::IO || errorCode == protocol::HAZELCAST_INSTANCE_NOT_ACTIVE || exception.is_retryable()) {
                        return true;
                    }
                    if (errorCode == protocol::TARGET_DISCONNECTED) {
                        return clientMessage_.load()->get()->is_retryable() || invocationService_.is_redo_operation();
                    }
                    return false;
                }

                std::ostream &operator<<(std::ostream &os, const ClientInvocation &invocation) {
                    std::ostringstream target;
                    if (invocation.is_bind_to_single_connection()) {
                        target << "connection " << *invocation.connection_;
                    } else if (invocation.partitionId_ != -1) {
                        target << "partition " << invocation.partitionId_;
                    } else if (!invocation.uuid_.is_nil()) {
                        target << "uuid " << boost::to_string(invocation.uuid_);
                    } else {
                        target << "random";
                    }
                    os << "ClientInvocation{" << "requestMessage = " << *invocation.clientMessage_.load()->get()
                       << ", objectName = "
                       << invocation.objectName_ << ", target = " << target.str() << ", sendConnection = ";
                    auto sendConnection = invocation.get_send_connection();
                    if (sendConnection) {
                        os << *sendConnection;
                    } else {
                        os << "nullptr";
                    }
                    os << '}';

                    return os;
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                           std::shared_ptr<protocol::ClientMessage> &&clientMessage,
                                                                           const std::string &objectName,
                                                                           int partitionId) {
                    return std::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, std::move(clientMessage), objectName, partitionId));
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                           std::shared_ptr<protocol::ClientMessage> &&clientMessage,
                                                                           const std::string &objectName,
                                                                           const std::shared_ptr<connection::Connection> &connection) {
                    return std::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, std::move(clientMessage), objectName, UNASSIGNED_PARTITION,
                                                 connection));
                }


                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                           std::shared_ptr<protocol::ClientMessage> &&clientMessage,
                                                                           const std::string &objectName,
                                                                           boost::uuids::uuid uuid) {
                    return std::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, std::move(clientMessage), objectName, UNASSIGNED_PARTITION,
                                                 nullptr, uuid));
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                           protocol::ClientMessage &clientMessage,
                                                                           const std::string &objectName,
                                                                           int partitionId) {
                    return create(clientContext, std::make_shared<protocol::ClientMessage>(std::move(clientMessage)),
                                  objectName, partitionId);
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                           protocol::ClientMessage &clientMessage,
                                                                           const std::string &objectName,
                                                                           const std::shared_ptr<connection::Connection> &connection) {
                    return create(clientContext, std::make_shared<protocol::ClientMessage>(std::move(clientMessage)),
                                  objectName, connection);
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                           protocol::ClientMessage &clientMessage,
                                                                           const std::string &objectName,
                                                                           boost::uuids::uuid uuid) {
                    return create(clientContext, std::make_shared<protocol::ClientMessage>(std::move(clientMessage)),
                                  objectName, uuid);
                }

                std::shared_ptr<connection::Connection> ClientInvocation::get_send_connection() const {
                    return *sendConnection_.load();
                }

                std::shared_ptr<connection::Connection> ClientInvocation::get_send_connection_or_wait() const {
                    while (!(*sendConnection_.load()) && lifecycleService_.is_running()) {
                        std::this_thread::yield();
                    }
                    if (!lifecycleService_.is_running()) {
                        BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("Client is being shut down!"));
                    }
                    return *sendConnection_.load();
                }

                void
                ClientInvocation::set_send_connection(const std::shared_ptr<connection::Connection> &conn) {
                    sendConnection_.store(boost::make_shared<std::shared_ptr<connection::Connection>>(conn));
                }

                void ClientInvocation::notify(const std::shared_ptr<protocol::ClientMessage> &msg) {
                    if (!msg) {
                        BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("response can't be null"));
                    }

                    int8_t expected_backups = msg->get_number_of_backups();

                    // if a regular response comes and there are backups, we need to wait for the backups
                    // when the backups complete, the response will be send by the last backup or backup-timeout-handle mechanism kicks on
                    if (expected_backups > backup_acks_received_) {
                        // so the invocation has backups and since not all backups have completed, we need to wait
                        // (it could be that backups arrive earlier than the response)

                        pending_response_received_time_ = std::chrono::steady_clock::now();

                        backup_acks_expected_ = expected_backups;

                        // it is very important that the response is set after the backupsAcksExpected is set, else the system
                        // can assume the invocation is complete because there is a response and no backups need to respond
                        pending_response_ = msg;

                        // we are done since not all backups have completed. Therefore we should not notify the future
                        return;
                    }

                    // we are going to notify the future that a response is available; this can happen when:
                    // - we had a regular operation (so no backups we need to wait for) that completed
                    // - we had a backup-aware operation that has completed, but also all its backups have completed
                    complete(msg);
                }

                void ClientInvocation::complete(const std::shared_ptr<protocol::ClientMessage> &msg) {
                    try {
                        // TODO: move msg content here?
                        this->invocationPromise_.set_value(*msg);
                    } catch (std::exception &e) {
                        HZ_LOG(logger_, warning,
                            boost::str(boost::format("Failed to set the response for invocation. "
                                                     "Dropping the response. %1%, %2% Response: %3%")
                                                     % e.what() % *this % *msg)
                        );
                    }
                    this->erase_invocation();
                }

                std::shared_ptr<protocol::ClientMessage> ClientInvocation::get_client_message() const {
                    return *clientMessage_.load();
                }

                const std::shared_ptr<EventHandler<protocol::ClientMessage> > &
                ClientInvocation::get_event_handler() const {
                    return eventHandler_;
                }

                void ClientInvocation::set_event_handler(
                        const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                    ClientInvocation::eventHandler_ = handler;
                }

                void ClientInvocation::execute() {
                    auto this_invocation = shared_from_this();
                    auto command = [=]() {
                        this_invocation->run();
                    };

                    // first we force a new invocation slot because we are going to return our old invocation slot immediately after
                    // It is important that we first 'force' taking a new slot; otherwise it could be that a sneaky invocation gets
                    // through that takes our slot!
                    int64_t callId = callIdSequence_->force_next();
                    clientMessage_.load()->get()->set_correlation_id(callId);

                    //we release the old slot
                    callIdSequence_->complete();

                    if (invokeCount_ < MAX_FAST_INVOCATION_COUNT) {
                        // fast retry for the first few invocations
                        executionService_->execute(command);
                    } else {
                        // progressive retry delay
                        int64_t delayMillis = util::min<int64_t>(static_cast<int64_t>(1) << (invokeCount_ - MAX_FAST_INVOCATION_COUNT),
                                                                 std::chrono::duration_cast<std::chrono::milliseconds>(retryPause_).count());
                        executionService_->schedule(command, std::chrono::milliseconds(delayMillis));
                    }
                }

                const std::string ClientInvocation::get_name() const {
                    return "ClientInvocation";
                }

                std::shared_ptr<protocol::ClientMessage> ClientInvocation::copy_message() {
                    return std::make_shared<protocol::ClientMessage>(**clientMessage_.load());
                }

                boost::promise<protocol::ClientMessage> &ClientInvocation::get_promise() {
                    return invocationPromise_;
                }

                void ClientInvocation::log_exception(exception::IException &e) {
                    HZ_LOG(logger_, finest,
                        boost::str(boost::format("Invocation got an exception %1%, invoke count : %2%, "
                                                 "exception : %3%")
                                                 % *this % invokeCount_.load() % e)
                    );
                }

                void ClientInvocation::notify_backup() {
                    ++backup_acks_received_;

                    if (!pending_response_) {
                        // no pendingResponse has been set, so we are done since the invocation on the primary needs to complete first
                        return;
                    }

                    // if a pendingResponse is set, then the backupsAcksExpected has been set (so we can now safely read backupsAcksExpected)
                    if (backup_acks_expected_ != backup_acks_received_) {
                        // we managed to complete a backup, but we were not the one completing the last backup, so we are done
                        return;
                    }

                    // we are the lucky one since we just managed to complete the last backup for this invocation and since the
                    // pendingResponse is set, we can set it on the future
                    complete_with_pending_response();
                }

                void
                ClientInvocation::detect_and_handle_backup_timeout(const std::chrono::milliseconds &backupTimeout) {
                    // if the backups have completed, we are done; this also filters out all non backup-aware operations
                    // since the backupsAcksExpected will always be equal to the backupsAcksReceived
                    if (backup_acks_expected_ == backup_acks_received_) {
                        return;
                    }

                    // if no response has yet been received, we we are done; we are only going to re-invoke an operation
                    // if the response of the primary has been received, but the backups have not replied
                    if (!pending_response_) {
                        return;
                    }

                    // if this has not yet expired (so has not been in the system for a too long period) we ignore it
                    if (pending_response_received_time_ + backupTimeout >= std::chrono::steady_clock::now()) {
                        return;
                    }

                    if (invocationService_.fail_on_indeterminate_state()) {
                        auto exception = boost::enable_current_exception((exception::ExceptionBuilder<exception::IndeterminateOperationStateException>(
                                "ClientInvocation::detect_and_handle_backup_timeout") << *this
                                                                                      << " failed because backup acks missed.").build());
                        notify_exception(std::make_exception_ptr(exception));
                        return;
                    }

                    // the backups have not yet completed, but we are going to release the future anyway if a pendingResponse has been set
                    complete_with_pending_response();
                }

                void ClientInvocation::complete_with_pending_response() {
                    complete(pending_response_);
                }

                ClientContext &impl::ClientTransactionManagerServiceImpl::get_client() const {
                    return client_;
                }

                ClientTransactionManagerServiceImpl::ClientTransactionManagerServiceImpl(ClientContext &client)
                        : client_(client) {}

                std::shared_ptr<connection::Connection> ClientTransactionManagerServiceImpl::connect() {
                    auto &invocationService = client_.get_invocation_service();
                    auto startTime = std::chrono::steady_clock::now();
                    auto invocationTimeout = invocationService.get_invocation_timeout();
                    ClientConfig &clientConfig = client_.get_client_config();
                    bool smartRouting = clientConfig.get_network_config().is_smart_routing();

                    while (client_.get_lifecycle_service().is_running()) {
                        try {
                            auto connection = client_.get_connection_manager().get_random_connection();
                            if (!connection) {
                                throw_exception(smartRouting);
                            }
                            return connection;
                        } catch (exception::HazelcastClientOfflineException &) {
                            throw;
                        } catch (exception::IException &) {
                            if (std::chrono::steady_clock::now() - startTime > invocationTimeout) {
                                std::rethrow_exception(
                                        new_operation_timeout_exception(std::current_exception(), invocationTimeout,
                                                                     startTime));
                            }
                        }
                        std::this_thread::sleep_for(invocationService.get_invocation_retry_pause());
                    }
                    BOOST_THROW_EXCEPTION(
                            exception::HazelcastClientNotActiveException("ClientTransactionManagerServiceImpl::connect",
                                                                         "Client is shutdown"));
                }

                std::exception_ptr
                ClientTransactionManagerServiceImpl::new_operation_timeout_exception(std::exception_ptr cause,
                                                                                  std::chrono::milliseconds invocationTimeout,
                                                                                  std::chrono::steady_clock::time_point startTime) {
                    std::ostringstream sb;
                    auto now = std::chrono::steady_clock::now();
                    sb
                            << "Creating transaction context timed out because exception occurred after client invocation timeout "
                            << std::chrono::duration_cast<std::chrono::milliseconds>(invocationTimeout).count() << " ms. " << "Current time: "
                            << util::StringUtil::time_to_string(std::chrono::steady_clock::now()) << ". " << "Start time: "
                            << util::StringUtil::time_to_string(startTime) << ". Total elapsed time: "
                            << std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() << " ms. ";
                    try {
                        std::rethrow_exception(cause);
                    } catch (...) {
                        try {
                            std::throw_with_nested(boost::enable_current_exception(exception::OperationTimeoutException(
                                    "ClientTransactionManagerServiceImpl::newOperationTimeoutException", sb.str())));
                        } catch (...) {
                            return std::current_exception();
                        }
                    }
                    return nullptr;
                }

                void ClientTransactionManagerServiceImpl::throw_exception(bool smart_routing) {
                    auto &client_config = client_.get_client_config();
                    auto &connection_strategy_Config = client_config.get_connection_strategy_config();
                    auto reconnect_mode = connection_strategy_Config.get_reconnect_mode();
                    if (reconnect_mode == config::ClientConnectionStrategyConfig::ReconnectMode::ASYNC) {
                        BOOST_THROW_EXCEPTION(exception::HazelcastClientOfflineException(
                                "ClientTransactionManagerServiceImpl::throw_exception", ""));
                    }
                    if (smart_routing) {
                        auto members = client_.get_cluster().get_members();
                        std::ostringstream msg;
                        if (members.empty()) {
                            msg << "No address was return by the LoadBalancer since there are no members in the cluster";
                        } else {
                            msg << "No address was return by the LoadBalancer. "
                                   "But the cluster contains the following members:{\n";
                            for (auto const &m : members) {
                                msg << '\t' << m << '\n';
                            }
                            msg << "}";
                        }
                        BOOST_THROW_EXCEPTION(exception::IllegalStateException(
                                                      "ClientTransactionManagerServiceImpl::throw_exception", msg.str()));
                    }
                    BOOST_THROW_EXCEPTION(exception::IllegalStateException(
                                                  "ClientTransactionManagerServiceImpl::throw_exception",
                                                  "No active connection is found"));
                }

                AwsAddressProvider::AwsAddressProvider(config::ClientAwsConfig &awsConfig, int awsMemberPort,
                                                       logger &lg) : awsMemberPort_(
                        util::IOUtil::to_string<int>(awsMemberPort)), logger_(lg), awsClient_(awsConfig, lg) {
                }

                std::vector<Address> AwsAddressProvider::load_addresses() {
                    update_lookup_table();
                    std::unordered_map<std::string, std::string> lookupTable = get_lookup_table();
                    std::vector<Address> addresses;

                    typedef std::unordered_map<std::string, std::string> LookupTable;
                    for (const LookupTable::value_type &privateAddress : lookupTable) {
                        std::vector<Address> possibleAddresses = util::AddressHelper::get_socket_addresses(
                                privateAddress.first + ":" + awsMemberPort_, logger_);
                        addresses.insert(addresses.begin(), possibleAddresses.begin(),
                                         possibleAddresses.end());
                    }
                    return addresses;
                }

                void AwsAddressProvider::update_lookup_table() {
                    try {
                        privateToPublic_ = awsClient_.get_addresses();
                    } catch (exception::IException &e) {
                        HZ_LOG(logger_, warning,
                            boost::str(boost::format("Aws addresses failed to load: %1%") % e.get_message())
                        );
                    }
                }

                std::unordered_map<std::string, std::string> AwsAddressProvider::get_lookup_table() {
                    return privateToPublic_;
                }

                AwsAddressProvider::~AwsAddressProvider() = default;

                Address DefaultAddressTranslator::translate(const Address &address) {
                    return address;
                }

                void DefaultAddressTranslator::refresh() {
                }


                ClientPartitionServiceImpl::ClientPartitionServiceImpl(ClientContext &client)
                        : client_(client), logger_(client.get_logger()), partitionCount_(0),
                        partition_table_(boost::shared_ptr<partition_table>(new partition_table{nullptr, -1})) {
                }

                void ClientPartitionServiceImpl::handle_event(const std::shared_ptr<connection::Connection>& connection, int32_t version,
                                                              const std::vector<std::pair<boost::uuids::uuid, std::vector<int>>> &partitions) {
                    HZ_LOG(logger_, finest,
                        boost::str(boost::format("Handling new partition table with partitionStateVersion: %1%") % version)
                    );

                    while (true) {
                        auto current = partition_table_.load();
                        if (!should_be_applied(connection, version, partitions, *current)) {
                            return;
                        }
                        if (partition_table_.compare_exchange_strong(current, boost::shared_ptr<partition_table>(
                                new partition_table{connection, version, convert_to_map(partitions)}))) {
                            HZ_LOG(logger_, finest,
                                boost::str(boost::format("Applied partition table with partitionStateVersion : %1%") % version)
                            );
                            return;
                        }

                    }
                }

                boost::uuids::uuid ClientPartitionServiceImpl::get_partition_owner(int32_t partitionId) {
                    auto table_ptr = partition_table_.load();
                    auto it = table_ptr->partitions.find(partitionId);
                    if (it != table_ptr->partitions.end()) {
                        return it->second;
                    }
                    return boost::uuids::nil_uuid();
                }

                int32_t ClientPartitionServiceImpl::get_partition_id(const serialization::pimpl::Data &key) {
                    int32_t pc = get_partition_count();
                    if (pc <= 0) {
                        return 0;
                    }
                    int hash = key.get_partition_hash();
                    return util::HashUtil::hash_to_index(hash, pc);
                }

                int32_t ClientPartitionServiceImpl::get_partition_count() {
                    return partitionCount_.load();
                }

                std::shared_ptr<client::impl::Partition> ClientPartitionServiceImpl::get_partition(int partitionId) {
                    return std::shared_ptr<client::impl::Partition>(new PartitionImpl(partitionId, client_, *this));
                }

                bool ClientPartitionServiceImpl::check_and_set_partition_count(int32_t new_partition_count) {
                    int32_t expected = 0;
                    if (partitionCount_.compare_exchange_strong(expected, new_partition_count)) {
                        return true;
                    }
                    return partitionCount_.load() == new_partition_count;
                }

                bool
                ClientPartitionServiceImpl::should_be_applied(const std::shared_ptr<connection::Connection> &connection,
                                                              int32_t version,
                                                              const std::vector<std::pair<boost::uuids::uuid, std::vector<int>>> &partitions,
                                                              const partition_table &current) {
                    auto &lg = client_.get_logger();
                    if (partitions.empty()) {
                        if (logger_.enabled(logger::level::finest)) {
                            log_failure(connection, version, current, "response is empty");
                        }
                        return false;
                    }
                    if (!current.connection || *connection != *current.connection) {
                        HZ_LOG(lg, finest, 
                            ([&current, &connection](){
                                auto frmt = boost::format("Event coming from a new connection. Old connection: %1%, "
                                                          "new connection %2%");

                                if (current.connection) {
                                    frmt = frmt % *current.connection;
                                } else {
                                    frmt = frmt % "nullptr";
                                }

                                return boost::str(frmt % *connection);
                            })()
                        );
                        
                        return true;
                    }
                    if (version <= current.version) {
                        if (lg.enabled(logger::level::finest)) {
                            log_failure(connection, version, current, "response state version is old");
                        }
                        return false;
                    }
                    return true;
                }

                void ClientPartitionServiceImpl::log_failure(const std::shared_ptr<connection::Connection> &connection,
                                                             int32_t version,
                                                             const ClientPartitionServiceImpl::partition_table &current,
                                                             const std::string &cause) {
                    HZ_LOG(logger_, finest,
                        [&](){
                            auto frmt = boost::format(" We will not apply the response, since %1% ."
                                                      " Response is from %2%. "
                                                      "Current connection %3%, response state version:%4%. "
                                                      "Current state version: %5%");
                            if (current.connection) {
                                return boost::str(frmt % cause % *connection % *current.connection % version % current.version);
                            }
                            else {
                                return boost::str(frmt % cause % *connection % "nullptr" % version % current.version);
                            }
                        }()
                    );
                }

                void ClientPartitionServiceImpl::reset() {
                    partition_table_.store(nullptr);
                }

                std::unordered_map<int32_t, boost::uuids::uuid> ClientPartitionServiceImpl::convert_to_map(
                        const std::vector<std::pair<boost::uuids::uuid, std::vector<int>>> &partitions) {
                    std::unordered_map<int32_t, boost::uuids::uuid> new_partitions;
                    for (auto const &e : partitions) {
                        for (auto pid: e.second) {
                            new_partitions.insert({pid, e.first});
                        }
                    }
                    return new_partitions;
                }

                int ClientPartitionServiceImpl::PartitionImpl::get_partition_id() const {
                    return partitionId_;
                }

                boost::optional<Member> ClientPartitionServiceImpl::PartitionImpl::get_owner() const {
                    auto owner = partitionService_.get_partition_owner(partitionId_);
                    if (!owner.is_nil()) {
                        return client_.get_client_cluster_service().get_member(owner);
                    }
                    return boost::none;
                }

                ClientPartitionServiceImpl::PartitionImpl::PartitionImpl(int partitionId, ClientContext &client,
                                                                         ClientPartitionServiceImpl &partitionService)
                        : partitionId_(partitionId), client_(client), partitionService_(partitionService) {
                }

                namespace sequence {
                    CallIdSequenceWithoutBackpressure::CallIdSequenceWithoutBackpressure() : head_(0) {}

                    CallIdSequenceWithoutBackpressure::~CallIdSequenceWithoutBackpressure() = default;

                    int32_t CallIdSequenceWithoutBackpressure::get_max_concurrent_invocations() const {
                        return INT32_MAX;
                    }

                    int64_t CallIdSequenceWithoutBackpressure::next() {
                        return force_next();
                    }

                    int64_t CallIdSequenceWithoutBackpressure::force_next() {
                        return ++head_;
                    }

                    void CallIdSequenceWithoutBackpressure::complete() {
                        // no-op
                    }

                    int64_t CallIdSequenceWithoutBackpressure::get_last_call_id() {
                        return head_;
                    }

                    // TODO: see if we can utilize std::hardware_destructive_interference_size
                    AbstractCallIdSequence::AbstractCallIdSequence(int32_t maxConcurrentInvocations) {
                        std::ostringstream out;
                        out << "maxConcurrentInvocations should be a positive number. maxConcurrentInvocations="
                            << maxConcurrentInvocations;
                        this->maxConcurrentInvocations_ = util::Preconditions::check_positive(maxConcurrentInvocations,
                                                                                            out.str());

                        for (size_t i = 0; i < longs_.size(); ++i) {
                            longs_[i] = 0;
                        }
                    }

                    AbstractCallIdSequence::~AbstractCallIdSequence() = default;

                    int32_t AbstractCallIdSequence::get_max_concurrent_invocations() const {
                        return maxConcurrentInvocations_;
                    }

                    int64_t AbstractCallIdSequence::next() {
                        if (!has_space()) {
                            handle_no_space_left();
                        }
                        return force_next();
                    }

                    int64_t AbstractCallIdSequence::force_next() {
                        return ++longs_[INDEX_HEAD];
                    }

                    void AbstractCallIdSequence::complete() {
                        ++longs_[INDEX_TAIL];
                        assert(longs_[INDEX_TAIL] <= longs_[INDEX_HEAD]);
                    }

                    int64_t AbstractCallIdSequence::get_last_call_id() {
                        return longs_[INDEX_HEAD];
                    }

                    bool AbstractCallIdSequence::has_space() {
                        return longs_[INDEX_HEAD] - longs_[INDEX_TAIL] < maxConcurrentInvocations_;
                    }

                    int64_t AbstractCallIdSequence::get_tail() {
                        return longs_[INDEX_TAIL];
                    }

                    const std::unique_ptr<util::concurrent::IdleStrategy> CallIdSequenceWithBackpressure::IDLER(
                            new util::concurrent::BackoffIdleStrategy(
                                    0, 0, std::chrono::duration_cast<std::chrono::nanoseconds>(
                                            std::chrono::microseconds(1000)).count(),
                                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                                            std::chrono::microseconds(MAX_DELAY_MS * 1000)).count()));

                    CallIdSequenceWithBackpressure::CallIdSequenceWithBackpressure(int32_t maxConcurrentInvocations,
                                                                                   int64_t backoffTimeoutMs)
                            : AbstractCallIdSequence(maxConcurrentInvocations) {
                        std::ostringstream out;
                        out << "backoffTimeoutMs should be a positive number. backoffTimeoutMs=" << backoffTimeoutMs;
                        util::Preconditions::check_positive(backoffTimeoutMs, out.str());

                        backoffTimeoutNanos_ = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                std::chrono::milliseconds(backoffTimeoutMs)).count();
                    }

                    void CallIdSequenceWithBackpressure::handle_no_space_left() {
                        auto start = std::chrono::steady_clock::now();
                        for (int64_t idleCount = 0;; idleCount++) {
                            int64_t elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                    std::chrono::steady_clock::now() - start).count();
                            if (elapsedNanos > backoffTimeoutNanos_) {
                                throw (exception::ExceptionBuilder<exception::HazelcastOverloadException>(
                                        "CallIdSequenceWithBackpressure::handleNoSpaceLeft")
                                        << "Timed out trying to acquire another call ID."
                                        << " maxConcurrentInvocations = " << get_max_concurrent_invocations()
                                        << ", backoffTimeout = "
                                        << std::chrono::microseconds(backoffTimeoutNanos_ / 1000).count()
                                        << " msecs, elapsed:"
                                        << std::chrono::microseconds(elapsedNanos / 1000).count() << " msecs").build();
                            }
                            IDLER->idle(idleCount);
                            if (has_space()) {
                                return;
                            }

                        }
                    }

                    FailFastCallIdSequence::FailFastCallIdSequence(int32_t maxConcurrentInvocations)
                            : AbstractCallIdSequence(maxConcurrentInvocations) {}

                    void FailFastCallIdSequence::handle_no_space_left() {
                        throw (exception::ExceptionBuilder<exception::HazelcastOverloadException>(
                                "FailFastCallIdSequence::handleNoSpaceLeft")
                                << "Maximum invocation count is reached. maxConcurrentInvocations = "
                                << get_max_concurrent_invocations()).build();

                    }

                    std::unique_ptr<CallIdSequence> CallIdFactory::new_call_id_sequence(bool isBackPressureEnabled,
                                                                                     int32_t maxAllowedConcurrentInvocations,
                                                                                     int64_t backoffTimeoutMs) {
                        if (!isBackPressureEnabled) {
                            return std::unique_ptr<CallIdSequence>(new CallIdSequenceWithoutBackpressure());
                        } else if (backoffTimeoutMs <= 0) {
                            return std::unique_ptr<CallIdSequence>(
                                    new FailFastCallIdSequence(maxAllowedConcurrentInvocations));
                        } else {
                            return std::unique_ptr<CallIdSequence>(
                                    new CallIdSequenceWithBackpressure(maxAllowedConcurrentInvocations,
                                                                       backoffTimeoutMs));
                        }
                    }
                }

                namespace listener {
                    listener_service_impl::listener_service_impl(ClientContext &clientContext,
                                                                 int32_t eventThreadCount)
                            : clientContext_(clientContext),
                              serializationService_(clientContext.get_serialization_service()),
                              logger_(clientContext.get_logger()),
                              clientConnectionManager_(clientContext.get_connection_manager()),
                              numberOfEventThreads_(eventThreadCount),
                              smart_(clientContext.get_client_config().get_network_config().is_smart_routing()) {
                        auto &invocationService = clientContext.get_invocation_service();
                        invocationTimeout_ = invocationService.get_invocation_timeout();
                        invocationRetryPause_ = invocationService.get_invocation_retry_pause();
                    }

                    bool listener_service_impl::registers_local_only() const {
                        return smart_;
                    }

                    boost::future<boost::uuids::uuid>
                    listener_service_impl::register_listener(
                            std::shared_ptr<ListenerMessageCodec> listenerMessageCodec,
                            std::shared_ptr<client::impl::BaseEventHandler> handler) {
                        auto task = boost::packaged_task<boost::uuids::uuid()>(
                                [=]() {
                                    return register_listener_internal(listenerMessageCodec, handler);
                                });
                        auto f = task.get_future();
                        boost::asio::post(registrationExecutor_->get_executor(), std::move(task));
                        return f;
                    }

                    boost::future<bool> listener_service_impl::deregister_listener(boost::uuids::uuid registrationId) {
                        util::Preconditions::check_not_nill(registrationId, "Nil userRegistrationId is not allowed!");

                        boost::packaged_task<bool()> task([=]() {
                            return deregister_listener_internal(registrationId);
                        });
                        auto f = task.get_future();
                        boost::asio::post(registrationExecutor_->get_executor(), std::move(task));
                        return f;
                    }

                    void listener_service_impl::connection_added(
                            const std::shared_ptr<connection::Connection> connection) {
                        boost::asio::post(registrationExecutor_->get_executor(), [=]() { connection_added_internal(connection); });
                    }

                    void listener_service_impl::connection_removed(
                            const std::shared_ptr<connection::Connection> connection) {
                        boost::asio::post(registrationExecutor_->get_executor(), [=]() { connection_removed_internal(connection); });
                    }

                    void
                    listener_service_impl::remove_event_handler(int64_t call_id,
                                                              const std::shared_ptr<connection::Connection> &connection) {
                        boost::asio::post(connection->get_socket().get_executor(),
                                          std::packaged_task<void()>([=]() {
                                              connection->deregister_invocation(call_id);
                                          }));
                    }

                    void listener_service_impl::handle_client_message(
                            const std::shared_ptr<ClientInvocation> invocation,
                            const std::shared_ptr<protocol::ClientMessage> response) {
                        try {
                            auto partitionId = response->get_partition_id();
                            if (partitionId == -1) {
                                // execute on random thread on the thread pool
                                boost::asio::post(eventExecutor_->get_executor(), [=]() { process_event_message(invocation, response); });
                                return;
                            }

                            // process on certain thread which is same for the partition id
                            boost::asio::post(eventStrands_[partitionId % eventStrands_.size()],
                                              [=]() { process_event_message(invocation, response); });

                        } catch (const std::exception &e) {
                            if (clientContext_.get_lifecycle_service().is_running()) {
                                HZ_LOG(logger_, warning,
                                    boost::str(boost::format("Delivery of event message to event handler failed. %1%, %2%, %3%")
                                                             % e.what() % *response % *invocation)
                                );
                            }
                        }
                    }

                    void listener_service_impl::shutdown() {
                        eventStrands_.clear();
                        ClientExecutionServiceImpl::shutdown_thread_pool(eventExecutor_.get());
                        ClientExecutionServiceImpl::shutdown_thread_pool(registrationExecutor_.get());
                    }

                    void listener_service_impl::start() {
                        eventExecutor_.reset(new hazelcast::util::hz_thread_pool(numberOfEventThreads_));
                        registrationExecutor_.reset(new hazelcast::util::hz_thread_pool(1));

                        for (int i = 0; i < numberOfEventThreads_; ++i) {
                            eventStrands_.emplace_back(eventExecutor_->get_executor());
                        }

                        clientConnectionManager_.add_connection_listener(shared_from_this());
                    }

                    boost::uuids::uuid listener_service_impl::register_listener_internal(
                            std::shared_ptr<ListenerMessageCodec> listenerMessageCodec,
                            std::shared_ptr<client::impl::BaseEventHandler> handler) {
                        auto user_registration_id = clientContext_.random_uuid();

                        std::shared_ptr<listener_registration> registration(new listener_registration{listenerMessageCodec, handler});
                        registrations_.put(user_registration_id, registration);
                        for (auto const &connection : clientConnectionManager_.get_active_connections()) {
                            try {
                                invoke(registration, connection);
                            } catch (exception::IException &e) {
                                if (connection->is_alive()) {
                                    deregister_listener_internal(user_registration_id);
                                    BOOST_THROW_EXCEPTION((exception::ExceptionBuilder<exception::HazelcastException>(
                                            "ClientListenerService::RegisterListenerTask::call")
                                            << "Listener can not be added " << e).build());
                                }
                            }
                        }
                        return user_registration_id;
                    }

                    bool
                    listener_service_impl::deregister_listener_internal(boost::uuids::uuid userRegistrationId) {
                        auto listenerRegistration = registrations_.get(userRegistrationId);
                        if (!listenerRegistration) {
                            return false;
                        }
                        bool successful = true;

                        auto listener_registrations = listenerRegistration->registrations.entry_set();
                        for (auto it = listener_registrations.begin();it != listener_registrations.end();) {
                            const auto &registration = it->second;
                            const auto& subscriber = it->first;
                            try {
                                const auto &listenerMessageCodec = listenerRegistration->codec;
                                auto serverRegistrationId = registration->server_registration_id;
                                auto request = listenerMessageCodec->encode_remove_request(serverRegistrationId);
                                auto invocation = ClientInvocation::create(clientContext_,request, "",
                                                                           subscriber);
                                invocation->invoke().get();

                                remove_event_handler(registration->call_id, subscriber);

                                it = listener_registrations.erase(it);
                            } catch (exception::IException &e) {
                                ++it;
                                if (subscriber->is_alive()) {
                                    successful = false;
                                    std::ostringstream endpoint;
                                    if (subscriber->get_remote_address()) {
                                        endpoint << *subscriber->get_remote_address();
                                    } else {
                                        endpoint << "null";
                                    }
                                    HZ_LOG(logger_, warning,
                                        boost::str(boost::format("ClientListenerService::deregisterListenerInternal "
                                                                 "Deregistration of listener with ID %1% "
                                                                 "has failed to address %2% %3%")
                                                                 % userRegistrationId 
                                                                 % subscriber->get_remote_address() % e)
                                    );
                                }
                            }
                        }
                        if (successful) {
                            registrations_.remove(userRegistrationId);
                        }
                        return successful;
                    }

                    void listener_service_impl::connection_added_internal(
                            const std::shared_ptr<connection::Connection> &connection) {
                        for (const auto &listener_registration : registrations_.values()) {
                            invoke_from_internal_thread(listener_registration, connection);
                        }
                    }

                    void listener_service_impl::connection_removed_internal(
                            const std::shared_ptr<connection::Connection> &connection) {
                        for (auto &registry : registrations_.values()) {
                            registry->registrations.remove(connection);
                        }
                    }

                    void
                    listener_service_impl::invoke_from_internal_thread(
                            const std::shared_ptr<listener_registration> &listener_registration,
                            const std::shared_ptr<connection::Connection> &connection) {
                        try {
                            invoke(listener_registration, connection);
                        } catch (exception::IException &e) {
                            HZ_LOG(logger_, warning,
                                boost::str(boost::format("Listener with pointer %1% can not be added to "
                                                         "a new connection: %2%, reason: %3%")
                                                         % listener_registration.get() % *connection % e)
                            );
                        }
                    }

                    void
                    listener_service_impl::invoke(const std::shared_ptr<listener_registration> &listener_registration,
                                                  const std::shared_ptr<connection::Connection> &connection) {
                        if (listener_registration->registrations.contains_key(connection)) {
                            return;
                        }

                        const auto &codec = listener_registration->codec;
                        auto request = codec->encode_add_request(registers_local_only());
                        const auto &handler = listener_registration->handler;
                        handler->before_listener_register();

                        auto invocation = ClientInvocation::create(clientContext_,
                                                                   std::make_shared<protocol::ClientMessage>(std::move(request)), "",
                                                                   connection);
                        invocation->set_event_handler(handler);
                        auto clientMessage = invocation->invoke_urgent().get();

                        auto serverRegistrationId = codec->decode_add_response(clientMessage);
                        handler->on_listener_register();
                        int64_t correlationId = invocation->get_client_message()->get_correlation_id();

                        (*listener_registration).registrations.put(connection, std::shared_ptr<connection_registration>(
                                new connection_registration{serverRegistrationId, correlationId}));
                    }

                    void listener_service_impl::process_event_message(
                            const std::shared_ptr<ClientInvocation> invocation,
                            const std::shared_ptr<protocol::ClientMessage> response) {
                        auto eventHandler = invocation->get_event_handler();
                        if (!eventHandler) {
                            if (clientContext_.get_lifecycle_service().is_running()) {
                                HZ_LOG(logger_, warning,
                                    boost::str(boost::format("No eventHandler for invocation. "
                                                             "Ignoring this invocation response. %1%")
                                                             % *invocation)
                                );
                            }

                            return;
                        }

                        try {
                            eventHandler->handle(*response);
                        } catch (std::exception &e) {
                            if (clientContext_.get_lifecycle_service().is_running()) {
                                HZ_LOG(logger_, warning,
                                    boost::str(boost::format("Delivery of event message to event handler failed. %1%, %2%, %3%")
                                                             % e.what() % *response % *invocation)
                                );
                            }
                        }
                    }

                    listener_service_impl::~listener_service_impl() = default;

                    void cluster_view_listener::start() {
                        client_context_.get_connection_manager().add_connection_listener(shared_from_this());
                    }

                    void cluster_view_listener::connection_added(const std::shared_ptr<connection::Connection> connection) {
                        try_register(connection);
                    }

                    void cluster_view_listener::connection_removed(const std::shared_ptr<connection::Connection> connection) {
                        try_reregister_to_random_connection(connection);
                    }

                    cluster_view_listener::cluster_view_listener(ClientContext &clientContext) : client_context_(
                            clientContext), listener_added_connection_(nullptr) {}

                    void cluster_view_listener::try_register(std::shared_ptr<connection::Connection> connection) {
                        connection::Connection *expected = nullptr;
                        if (!listener_added_connection_.compare_exchange_strong(expected, connection.get())) {
                            //already registering/registered to another connection
                            return;
                        }

                        auto invocation = ClientInvocation::create(client_context_,
                                 std::make_shared<protocol::ClientMessage>(
                                 protocol::codec::client_addclusterviewlistener_encode()), "", connection);

                        auto handler = std::shared_ptr<event_handler>(new event_handler(connection, *this));
                        invocation->set_event_handler(handler);
                        handler->before_listener_register();

                        invocation->invoke_urgent().then([=] (boost::future<protocol::ClientMessage> f) {
                            if (f.has_value()) {
                                handler->on_listener_register();
                                return;
                            }
                            //completes with exception, listener needs to be reregistered
                            try_reregister_to_random_connection(connection);
                        });

                    }

                    void cluster_view_listener::try_reregister_to_random_connection(
                            std::shared_ptr<connection::Connection> old_connection) {
                        auto conn_ptr = old_connection.get();
                        if (!listener_added_connection_.compare_exchange_strong(conn_ptr, nullptr)) {
                            //somebody else already trying to reregister
                            return;
                        }
                        auto new_connection = client_context_.get_connection_manager().get_random_connection();
                        if (new_connection) {
                            try_register(new_connection);
                        }
                    }

                    cluster_view_listener::~cluster_view_listener() = default;

                    void
                    cluster_view_listener::event_handler::handle_membersview(int32_t version,
                                                                             const std::vector<Member> &memberInfos) {
                        view_listener.client_context_.get_client_cluster_service().handle_event(version, memberInfos);
                    }

                    void
                    cluster_view_listener::event_handler::handle_partitionsview(int32_t version,
                                                                                const std::vector<std::pair<boost::uuids::uuid, std::vector<int>>> &partitions) {
                        view_listener.client_context_.get_partition_service().handle_event(connection, version, partitions);
                    }

                    void cluster_view_listener::event_handler::before_listener_register() {
                        view_listener.client_context_.get_client_cluster_service().clear_member_list_version();
                        auto &lg = view_listener.client_context_.get_logger();
                        HZ_LOG(lg, finest,
                            boost::str(boost::format("Register attempt of ClusterViewListenerHandler to %1%")
                                                    % *connection)
                        );
                    }

                    void cluster_view_listener::event_handler::on_listener_register() {
                        auto &lg = view_listener.client_context_.get_logger();
                        HZ_LOG(lg, finest,
                            boost::str(boost::format("Registered ClusterViewListenerHandler to %1%") 
                                                     % *connection)
                        );
                    }

                    cluster_view_listener::event_handler::event_handler(
                            const std::shared_ptr<connection::Connection> &connection,
                            cluster_view_listener &viewListener) : connection(connection),
                                                                   view_listener(viewListener) {}
                }

                protocol::ClientMessage
                ClientInvocationServiceImpl::BackupListenerMessageCodec::encode_add_request(bool localOnly) const {
                    return protocol::codec::client_localbackuplistener_encode();
                }

                protocol::ClientMessage ClientInvocationServiceImpl::BackupListenerMessageCodec::encode_remove_request(
                        boost::uuids::uuid realRegistrationId) const {
                    assert(0);
                    return protocol::ClientMessage(0);
                }

                void ClientInvocationServiceImpl::noop_backup_event_handler::handle_backup(
                        int64_t sourceInvocationCorrelationId) {
                    assert(0);
                }
            }
        }
    }
}

namespace std {
    bool less<hazelcast::client::spi::DefaultObjectNamespace>::operator()(
            const hazelcast::client::spi::DefaultObjectNamespace &lhs,
            const hazelcast::client::spi::DefaultObjectNamespace &rhs) const {
        int result = lhs.get_service_name().compare(rhs.get_service_name());
        if (result < 0) {
            return true;
        }

        if (result > 0) {
            return false;
        }

        return lhs.get_object_name().compare(rhs.get_object_name()) < 0;
    }

    std::size_t
    hash<hazelcast::client::spi::DefaultObjectNamespace>::operator()(
            const hazelcast::client::spi::DefaultObjectNamespace &k) const noexcept {
        return std::hash<std::string>()(k.get_service_name() + k.get_object_name());
    }
}



