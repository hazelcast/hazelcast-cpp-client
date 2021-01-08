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

#include "hazelcast/client/hazelcast_client.h"
#include <hazelcast/client/protocol/codec/ErrorCodec.h>
#include <hazelcast/client/spi/impl/ListenerMessageCodec.h>
#include <hazelcast/client/spi/impl/ClientClusterServiceImpl.h>
#include <hazelcast/client/spi/impl/listener/cluster_view_listener.h>
#include <hazelcast/client/spi/impl/listener/listener_service_impl.h>
#include "hazelcast/client/member_selectors.h"
#include "hazelcast/client/lifecycle_event.h"
#include "hazelcast/client/initial_membership_event.h"
#include "hazelcast/client/membership_event.h"
#include "hazelcast/client/lifecycle_listener.h"
#include "hazelcast/client/spi/ProxyManager.h"
#include "hazelcast/client/spi/ClientProxy.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/impl/ClientInvocationServiceImpl.h"
#include "hazelcast/client/impl/hazelcast_client_instance_impl.h"
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
#include "hazelcast/client/member_selectors.h"

namespace hazelcast {
    namespace client {
        const std::unordered_set<member> &initial_membership_event::get_members() const {
            return members_;
        }

        cluster &initial_membership_event::get_cluster() {
            return cluster_;
        }

        initial_membership_event::initial_membership_event(cluster &cluster, std::unordered_set<member> members) : cluster_(
                cluster), members_(std::move(members)) {
        }

        lifecycle_event::lifecycle_event(lifecycle_state state)
                : state_(state) {
        }

        lifecycle_event::lifecycle_state lifecycle_event::get_state() const {
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

            boost::future<void> ProxyManager::initialize(const std::shared_ptr<ClientProxy> &client_proxy) {
                auto clientMessage = protocol::codec::client_createproxy_encode(client_proxy->get_name(),
                        client_proxy->get_service_name());
                return spi::impl::ClientInvocation::create(client_, clientMessage,
                                                           client_proxy->get_service_name())->invoke().then(
                        boost::launch::deferred, [=](boost::future<protocol::ClientMessage> f) {
                            f.get();
                            client_proxy->on_initialize();
                        });
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
                        } catch (exception::iexception &) {
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

            ClientContext::ClientContext(const client::hazelcast_client &hazelcast_client) : hazelcast_client_(
                    *hazelcast_client.client_impl_) {
            }

            ClientContext::ClientContext(client::impl::hazelcast_client_instance_impl &hazelcast_client)
                    : hazelcast_client_(hazelcast_client) {
            }

            serialization::pimpl::SerializationService &ClientContext::get_serialization_service() {
                return hazelcast_client_.serialization_service_;
            }

            impl::ClientClusterServiceImpl & ClientContext::get_client_cluster_service() {
                return hazelcast_client_.cluster_service_;
            }

            impl::ClientInvocationServiceImpl &ClientContext::get_invocation_service() {
                return *hazelcast_client_.invocation_service_;
            }

            client_config &ClientContext::get_client_config() {
                return hazelcast_client_.client_config_;
            }

            impl::ClientPartitionServiceImpl & ClientContext::get_partition_service() {
                return *hazelcast_client_.partition_service_;
            }

            lifecycle_service &ClientContext::get_lifecycle_service() {
                return hazelcast_client_.lifecycle_service_;
            }

            spi::impl::listener::listener_service_impl &ClientContext::get_client_listener_service() {
                return *hazelcast_client_.listener_service_;
            }

            connection::ClientConnectionManagerImpl &ClientContext::get_connection_manager() {
                return *hazelcast_client_.connection_manager_;
            }

            internal::nearcache::NearCacheManager &ClientContext::get_near_cache_manager() {
                return *hazelcast_client_.near_cache_manager_;
            }

            client_properties &ClientContext::get_client_properties() {
                return hazelcast_client_.client_properties_;
            }

            cluster &ClientContext::get_cluster() {
                return hazelcast_client_.cluster_;
            }

            std::shared_ptr<impl::sequence::CallIdSequence> &ClientContext::get_call_id_sequence() const {
                return hazelcast_client_.call_id_sequence_;
            }

            const protocol::ClientExceptionFactory &ClientContext::get_client_exception_factory() const {
                return hazelcast_client_.get_exception_factory();
            }

            const std::string &ClientContext::get_name() const {
                return hazelcast_client_.get_name();
            }

            impl::ClientExecutionServiceImpl &ClientContext::get_client_execution_service() const {
                return *hazelcast_client_.execution_service_;
            }

            const std::shared_ptr<client::impl::ClientLockReferenceIdGenerator> &
            ClientContext::get_lock_reference_id_generator() {
                return hazelcast_client_.get_lock_reference_id_generator();
            }

            std::shared_ptr<client::impl::hazelcast_client_instance_impl>
            ClientContext::get_hazelcast_client_implementation() {
                return hazelcast_client_.shared_from_this();
            }

            spi::ProxyManager &ClientContext::get_proxy_manager() {
                return hazelcast_client_.get_proxy_manager();
            }

            logger &ClientContext::get_logger() {
                return *hazelcast_client_.logger_;
            }

            client::impl::statistics::Statistics &ClientContext::get_clientstatistics() {
                return *hazelcast_client_.statistics_;
            }

            spi::impl::listener::cluster_view_listener &ClientContext::get_cluster_view_listener() {
                return *hazelcast_client_.cluster_listener_;
            }

            boost::uuids::uuid ClientContext::random_uuid() {
                return hazelcast_client_.random_uuid();
            }

            cp::internal::session::proxy_session_manager &ClientContext::get_proxy_session_manager() {
                return hazelcast_client_.proxy_session_manager_;
            }

            lifecycle_service::lifecycle_service(ClientContext &client_context,
                                                 const std::vector<lifecycle_listener> &listeners) :
                    client_context_(client_context), listeners_() {
                for (const auto &listener: listeners) {
                    add_listener(lifecycle_listener(listener));
                }
            }

            bool lifecycle_service::is_started() const {
                return started_;
            }

            boost::future<bool> lifecycle_service::start() {
                return boost::async([=]() {
                    std::lock_guard<std::mutex> guard(start_stop_lock_);

                    if (active_) {
                        throw exception::illegal_state("lifecycle_service::start",
                                                       "Client is already started.");
                    }

                    active_ = true;

                    fire_lifecycle_event(lifecycle_event::STARTING);

                    fire_lifecycle_event(lifecycle_event::STARTED);

                    client_context_.get_client_execution_service().start();

                    client_context_.get_client_listener_service().start();

                    client_context_.get_invocation_service().start();

                    client_context_.get_client_cluster_service().start();

                    client_context_.get_cluster_view_listener().start();

                    if (!client_context_.get_connection_manager().start()) {
                        return false;
                    }

                    auto &connectionStrategyConfig = client_context_.get_client_config().get_connection_strategy_config();
                    if (!connectionStrategyConfig.is_async_start()) {
                        // The client needs to open connections to all members before any services requiring internal listeners start
                        wait_for_initial_membership_event();
                        client_context_.get_connection_manager().connect_to_all_cluster_members();
                    }

                    client_context_.get_invocation_service().add_backup_listener();

                    client_context_.get_clientstatistics().start();

                    started_ = true;

                    return true;
                });
            }

            boost::future<void> lifecycle_service::stop() {
                return boost::async([=]() {
                    std::lock_guard<std::mutex> guard(start_stop_lock_);

                    if (!active_) {
                        return;
                    }

                    active_ = false;

                    try {
                        fire_lifecycle_event(lifecycle_event::SHUTTING_DOWN);
                        client_context_.get_proxy_session_manager().shutdown();
                        client_context_.get_clientstatistics().shutdown();
                        client_context_.get_proxy_manager().destroy();
                        client_context_.get_connection_manager().shutdown();
                        client_context_.get_client_cluster_service().shutdown();
                        client_context_.get_invocation_service().shutdown();
                        client_context_.get_client_listener_service().shutdown();
                        client_context_.get_near_cache_manager().destroy_all_near_caches();
                        fire_lifecycle_event(lifecycle_event::SHUTDOWN);
                        client_context_.get_client_execution_service().shutdown();
                        client_context_.get_serialization_service().dispose();
                    } catch (std::exception &e) {
                        HZ_LOG(client_context_.get_logger(), info,
                               boost::str(boost::format("An exception occured during LifecycleService stop. %1%")
                                          % e.what())
                        );
                    }

                    started_ = false;
                });
            }

            boost::uuids::uuid lifecycle_service::add_listener(lifecycle_listener &&lifecycle_listener) {
                std::lock_guard<std::mutex> lg(listener_lock_);
                const auto id = uuid_generator_();
                listeners_.emplace(id, std::move(lifecycle_listener));
                return id;
            }

            bool lifecycle_service::remove_listener(const boost::uuids::uuid &registration_id) {
                std::lock_guard<std::mutex> guard(listener_lock_);
                return listeners_.erase(registration_id) == 1;
            }

            void lifecycle_service::fire_lifecycle_event(const lifecycle_event &lifecycle_event) {
                std::lock_guard<std::mutex> guard(listener_lock_);
                logger &lg = client_context_.get_logger();

                std::function<void(lifecycle_listener &)> fire_one;

                switch (lifecycle_event.get_state()) {
                    case lifecycle_event::STARTING : {
                        // convert the date string from "2016-04-20" to 20160420
                        std::string date(HAZELCAST_STRINGIZE(HAZELCAST_GIT_COMMIT_DATE));
                        util::git_date_to_hazelcast_log_date(date);
                        std::string commitId(HAZELCAST_STRINGIZE(HAZELCAST_GIT_COMMIT_ID));
                        commitId.erase(std::remove(commitId.begin(), commitId.end(), '"'), commitId.end());

                        HZ_LOG(lg, info,
                               (boost::format("(%1%:%2%) LifecycleService::LifecycleEvent Client (%3%) is STARTING") %
                                date % commitId % client_context_.get_connection_manager().get_client_uuid()).str());
                        char msg[100];
                        util::hz_snprintf(msg, 100, "(%s:%s) LifecycleService::LifecycleEvent STARTING", date.c_str(),
                                          commitId.c_str());
                        HZ_LOG(lg, info, msg);

                        fire_one = [](lifecycle_listener &listener) {
                            listener.starting_();
                        };
                        break;
                    }
                    case lifecycle_event::STARTED : {
                        HZ_LOG(lg, info, "LifecycleService::LifecycleEvent STARTED");

                        fire_one = [](lifecycle_listener &listener) {
                            listener.started_();
                        };
                        break;
                    }
                    case lifecycle_event::SHUTTING_DOWN : {
                        HZ_LOG(lg, info, "LifecycleService::LifecycleEvent SHUTTING_DOWN");

                        fire_one = [](lifecycle_listener &listener) {
                            listener.shutting_down_();
                        };
                        break;
                    }
                    case lifecycle_event::SHUTDOWN : {
                        HZ_LOG(lg, info, "LifecycleService::LifecycleEvent SHUTDOWN");

                        fire_one = [](lifecycle_listener &listener) {
                            listener.shutdown_();
                        };
                        break;
                    }
                    case lifecycle_event::CLIENT_CONNECTED : {
                        HZ_LOG(lg, info, "LifecycleService::LifecycleEvent CLIENT_CONNECTED");

                        fire_one = [](lifecycle_listener &listener) {
                            listener.connected_();
                        };
                        break;
                    }
                    case lifecycle_event::CLIENT_DISCONNECTED : {
                        HZ_LOG(lg, info, "LifecycleService::LifecycleEvent CLIENT_DISCONNECTED");

                        fire_one = [](lifecycle_listener &listener) {
                            listener.disconnected_();
                        };
                        break;
                    }
                }

                for (auto &item: listeners_) {
                    fire_one(item.second);
                }
            }

            bool lifecycle_service::is_running() {
                return active_;
            }

            lifecycle_service::~lifecycle_service() {
                if (active_) {
                    stop();
                }
            }

            void lifecycle_service::wait_for_initial_membership_event() const {
                client_context_.get_client_cluster_service().wait_initial_member_list_fetched();
            }

            DefaultObjectNamespace::DefaultObjectNamespace(const std::string &service, const std::string &object)
                    : service_name_(service), object_name_(object) {

            }

            const std::string &DefaultObjectNamespace::get_service_name() const {
                return service_name_;
            }

            const std::string &DefaultObjectNamespace::get_object_name() const {
                return object_name_;
            }

            bool DefaultObjectNamespace::operator==(const DefaultObjectNamespace &rhs) const {
                return service_name_ == rhs.service_name_ && object_name_ == rhs.object_name_;
            }

            ClientProxy::ClientProxy(const std::string &name, const std::string &service_name, ClientContext &context)
                    : name_(name), service_name_(service_name), context_(context) {}

            ClientProxy::~ClientProxy() = default;

            const std::string &ClientProxy::get_name() const {
                return name_;
            }

            const std::string &ClientProxy::get_service_name() const {
                return service_name_;
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
                    } catch (exception::iexception &) {
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
            ClientProxy::register_listener(std::shared_ptr<impl::ListenerMessageCodec> listener_message_codec,
                                          std::shared_ptr<client::impl::BaseEventHandler> handler) {
                handler->set_logger(&get_context().get_logger());
                return get_context().get_client_listener_service().register_listener(listener_message_codec,
                                                                                handler);
            }

            boost::future<bool> ClientProxy::deregister_listener(boost::uuids::uuid registration_id) {
                return get_context().get_client_listener_service().deregister_listener(registration_id);
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
                          invocation_timeout_(std::chrono::seconds(client.get_client_properties().get_integer(
                                  client.get_client_properties().get_invocation_timeout_seconds()))),
                          invocation_retry_pause_(std::chrono::milliseconds(client.get_client_properties().get_long(
                                  client.get_client_properties().get_invocation_retry_pause_millis()))),
                          response_thread_(logger_, *this, client),
                          smart_routing_(client.get_client_config().get_network_config().is_smart_routing()),
                          backup_acks_enabled_(smart_routing_ && client.get_client_config().backup_acks_enabled()),
                          fail_on_indeterminate_operation_state_(client.get_client_properties().get_boolean(client.get_client_properties().fail_on_indeterminate_state())),
                          backup_timeout_(std::chrono::milliseconds(client.get_client_properties().get_integer(client.get_client_properties().backup_timeout_millis()))) {}

                void ClientInvocationServiceImpl::start() {
                    response_thread_.start();
                }

                void ClientInvocationServiceImpl::add_backup_listener() {
                    if (this->backup_acks_enabled_) {
                        auto &listener_service = this->client_.get_client_listener_service();
                        listener_service.register_listener(std::make_shared<BackupListenerMessageCodec>(),
                                                          std::make_shared<noop_backup_event_handler>()).get();
                    }
                }

                void ClientInvocationServiceImpl::shutdown() {
                    is_shutdown_.store(true);

                    response_thread_.shutdown();
                }

                std::chrono::milliseconds ClientInvocationServiceImpl::get_invocation_timeout() const {
                    return invocation_timeout_;
                }

                std::chrono::milliseconds ClientInvocationServiceImpl::get_invocation_retry_pause() const {
                    return invocation_retry_pause_;
                }

                bool ClientInvocationServiceImpl::is_redo_operation() {
                    return client_.get_client_config().is_redo_operation();
                }

                void
                ClientInvocationServiceImpl::handle_client_message(const std::shared_ptr<ClientInvocation> &invocation,
                                                                   const std::shared_ptr<protocol::ClientMessage> &response) {
                    response_thread_.process(invocation, response);
                }

                bool ClientInvocationServiceImpl::send(const std::shared_ptr<impl::ClientInvocation>& invocation,
                                                           const std::shared_ptr<connection::Connection>& connection) {
                    if (is_shutdown_) {
                        BOOST_THROW_EXCEPTION(
                                exception::hazelcast_client_not_active("ClientInvocationServiceImpl::send",
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
                                                                        const std::shared_ptr<ClientInvocation> &client_invocation) {
                    auto clientMessage = client_invocation->get_client_message();
                    connection.write(client_invocation);
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
                                                                                  ClientInvocationServiceImpl &invocation_service,
                                                                                  ClientContext &client_context)
                        : logger_(lg), client_(client_context) {
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
                    client_properties &clientProperties = client_.get_client_properties();
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
                        const std::shared_ptr<protocol::ClientMessage> &response) {
                    if (!pool_) {
                        process_internal(invocation, response);
                        return;
                    }

                    boost::asio::post(pool_->get_executor(), [=] { process_internal(invocation, response); });
                }

                DefaultAddressProvider::DefaultAddressProvider(config::client_network_config &network_config,
                                                               bool no_other_address_provider_exist) : network_config_(
                        network_config), no_other_address_provider_exist_(no_other_address_provider_exist) {
                }

                std::vector<address> DefaultAddressProvider::load_addresses() {
                    std::vector<address> addresses = network_config_.get_addresses();
                    if (addresses.empty() && no_other_address_provider_exist_) {
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
                        membership_listener &&listener) {
                    std::lock_guard<std::mutex> g(listeners_lock_);
                    auto id = client_.random_uuid();
                    listeners_.emplace(id, std::move(listener));
                    return id;
                }

                boost::optional<member> ClientClusterServiceImpl::get_member(boost::uuids::uuid uuid) const {
                    assert(!uuid.is_nil());
                    auto members_view_ptr = member_list_snapshot_.load();
                    const auto it = members_view_ptr->members.find(uuid);
                    if (it == members_view_ptr->members.end()) {
                        return boost::none;
                    }
                    return {it->second};
                }

                std::vector<member> ClientClusterServiceImpl::get_member_list() const {
                    auto members_view_ptr = member_list_snapshot_.load();
                    std::vector<member> result;
                    result.reserve(members_view_ptr->members.size());
                    for (const auto &e : members_view_ptr->members) {
                        result.emplace_back(e.second);
                    }
                    return result;
                }

                void ClientClusterServiceImpl::start() {
                    for (auto &listener : client_.get_client_config().get_membership_listeners()) {
                        add_membership_listener(membership_listener(listener));
                    }
                }

                void ClientClusterServiceImpl::fire_initial_membership_event(const initial_membership_event &event) {
                    std::lock_guard<std::mutex> g(listeners_lock_);

                    for (auto &item : listeners_) {
                        membership_listener &listener = item.second;
                        if (listener.init_) {
                            listener.init_(event);
                        }
                    }
                }

                void ClientClusterServiceImpl::shutdown() {
                    initial_list_fetched_latch_.try_count_down();
                }

                boost::uuids::uuid
                ClientClusterServiceImpl::add_membership_listener(membership_listener &&listener) {
                    std::lock_guard<std::mutex> cluster_view_g(cluster_view_lock_);

                    auto id = add_membership_listener_without_init(std::move(listener));

                    std::lock_guard<std::mutex> listeners_g(listeners_lock_);
                    auto added_listener = listeners_[id];

                    if (added_listener.init_) {
                        auto &cluster = client_.get_cluster();
                        auto members_ptr = member_list_snapshot_.load();
                        if (!members_ptr->members.empty()) {
                            std::unordered_set<member> members;
                            for (const auto &e : members_ptr->members) {
                                members.insert(e.second);
                            }
                            added_listener.init_(initial_membership_event(cluster, members));
                        }
                    }

                    return id;
                }

                bool ClientClusterServiceImpl::remove_membership_listener(boost::uuids::uuid registration_id) {
                    std::lock_guard<std::mutex> g(listeners_lock_);
                    return listeners_.erase(registration_id) == 1;
                }

                std::vector<member>
                ClientClusterServiceImpl::get_members(const member_selector &selector) const {
                    std::vector<member> result;
                    for (auto &&member : get_member_list()) {
                        if (selector.select(member)) {
                            result.emplace_back(std::move(member));
                        }
                    }

                    return result;
                }

                local_endpoint ClientClusterServiceImpl::get_local_client() const {
                    connection::ClientConnectionManagerImpl &cm = client_.get_connection_manager();
                    auto connection = cm.get_random_connection();
                    auto inetSocketAddress = connection ? connection->get_local_socket_address() : boost::none;
                    auto uuid = cm.get_client_uuid();
                    return local_endpoint(uuid, std::move(inetSocketAddress), client_.get_name(), labels_);
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
                ClientClusterServiceImpl::handle_event(int32_t version, const std::vector<member> &member_infos) {
                    auto &lg = client_.get_logger();
                    HZ_LOG(lg, finest, 
                        boost::str(boost::format("Handling new snapshot with membership version: %1%, "
                                                 "membersString %2%")
                                                 % version
                                                 % members_string(create_snapshot(version, member_infos)))
                    );
                    auto cluster_view_snapshot = member_list_snapshot_.load();
                    if (cluster_view_snapshot == EMPTY_SNAPSHOT) {
                        std::lock_guard<std::mutex> g(cluster_view_lock_);
                        cluster_view_snapshot = member_list_snapshot_.load();
                        if (cluster_view_snapshot == EMPTY_SNAPSHOT) {
                            //this means this is the first time client connected to cluster
                            apply_initial_state(version, member_infos);
                            initial_list_fetched_latch_.count_down();
                            return;
                        }
                    }

                    std::vector<membership_event> events;
                    if (version >= cluster_view_snapshot->version) {
                        std::lock_guard<std::mutex> g(cluster_view_lock_);
                        cluster_view_snapshot = member_list_snapshot_.load();
                        if (version >= cluster_view_snapshot->version) {
                            auto prev_members = cluster_view_snapshot->members;
                            auto snapshot = boost::make_shared<member_list_snapshot>(create_snapshot(version, member_infos));
                            member_list_snapshot_.store(snapshot);
                            events = detect_membership_events(prev_members, snapshot->members);
                        }
                    }

                    fire_events(std::move(events));
                }

                ClientClusterServiceImpl::member_list_snapshot
                ClientClusterServiceImpl::create_snapshot(int32_t version, const std::vector<member> &members) {
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
                ClientClusterServiceImpl::apply_initial_state(int32_t version, const std::vector<member> &member_infos) {
                    auto snapshot = boost::make_shared<member_list_snapshot>(create_snapshot(version, member_infos));
                    member_list_snapshot_.store(snapshot);
                    HZ_LOG(client_.get_logger(), info, members_string(*snapshot));
                    std::unordered_set<member> members;
                    for(auto const &e : snapshot->members) {
                        members.insert(e.second);
                    }
                    std::lock_guard<std::mutex> g(listeners_lock_);
                    for (auto &item : listeners_) {
                        membership_listener &listener = item.second;
                        if (listener.init_) {
                            listener.init_(initial_membership_event(client_.get_cluster(), members));
                        }
                    }
                }

                std::vector<membership_event> ClientClusterServiceImpl::detect_membership_events(
                        std::unordered_map<boost::uuids::uuid, member, boost::hash<boost::uuids::uuid>> previous_members,
                        const std::unordered_map<boost::uuids::uuid, member, boost::hash<boost::uuids::uuid>>& current_members) {
                    std::vector<member> new_members;

                    for (auto const & e : current_members) {
                        if (!previous_members.erase(e.first)) {
                            new_members.emplace_back(e.second);
                        }
                    }

                    std::vector<membership_event> events;

                    // removal events should be added before added events
                    for (auto const &e : previous_members) {
                        events.emplace_back(client_.get_cluster(), e.second, membership_event::membership_event_type::MEMBER_LEFT, current_members);
                        auto connection = client_.get_connection_manager().get_connection(e.second.get_uuid());
                        if (connection) {
                            connection->close("", std::make_exception_ptr(exception::target_disconnected(
                                    "ClientClusterServiceImpl::detect_membership_events", (boost::format(
                                            "The client has closed the connection to this member, after receiving a member left event from the cluster. %1%") %
                                                                                           *connection).str())));
                        }
                    }
                    for (auto const &member : new_members) {
                        events.emplace_back(client_.get_cluster(), member, membership_event::membership_event_type::MEMBER_JOINED, current_members);
                    }

                    if (!events.empty()) {
                        auto snapshot = member_list_snapshot_.load();
                        if (!snapshot->members.empty()) {
                            HZ_LOG(client_.get_logger(), info, members_string(*snapshot));
                        }
                    }
                    return events;
                }

                void ClientClusterServiceImpl::fire_events(std::vector<membership_event> events) {
                    std::lock_guard<std::mutex> g(listeners_lock_);
                    
                    for (auto const &event : events) {
                        for (auto &item : listeners_) {
                            membership_listener &listener = item.second;
                            if (event.get_event_type() == membership_event::membership_event_type::MEMBER_JOINED) {
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
                        BOOST_THROW_EXCEPTION(exception::illegal_state(
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
                        const std::shared_ptr<ClientInvocation> &invocation, int partition_id) {
                    auto partition_owner = client_.get_partition_service().get_partition_owner(partition_id);
                    if (partition_owner.is_nil()) {
                        HZ_LOG(logger_, finest, 
                            boost::str(boost::format("Partition owner is not assigned yet for partition %1%")
                                                     % partition_id)
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
                                                                       const client_properties &properties,
                                                                       int32_t pool_size,
                                                                       spi::lifecycle_service &service)
                        : lifecycle_service_(service), client_properties_(properties), user_executor_pool_size_(pool_size) {}

                void ClientExecutionServiceImpl::start() {
                    int internalPoolSize = client_properties_.get_integer(client_properties_.get_internal_executor_pool_size());
                    if (internalPoolSize <= 0) {
                        internalPoolSize = util::IOUtil::to_value<int>(
                                client_properties::INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT);
                    }

                    if (user_executor_pool_size_ <= 0) {
                        user_executor_pool_size_ = std::thread::hardware_concurrency();
                    }
                    if (user_executor_pool_size_ <= 0) {
                        user_executor_pool_size_ = 4; // hard coded thread pool count in case we could not get the processor count
                    }

                    internal_executor_.reset(new hazelcast::util::hz_thread_pool(internalPoolSize));
                    user_executor_.reset(new hazelcast::util::hz_thread_pool(user_executor_pool_size_));
                }

                void ClientExecutionServiceImpl::shutdown() {
                    shutdown_thread_pool(user_executor_.get());
                    shutdown_thread_pool(internal_executor_.get());
                }

                boost::asio::thread_pool::executor_type ClientExecutionServiceImpl::get_user_executor() const {
                    return user_executor_->get_executor();
                }

                void ClientExecutionServiceImpl::shutdown_thread_pool(hazelcast::util::hz_thread_pool *pool) {
                    if (!pool) {
                        return;
                    }
                    pool->shutdown_gracefully();
                }

                constexpr int ClientInvocation::MAX_FAST_INVOCATION_COUNT;
                constexpr int ClientInvocation::UNASSIGNED_PARTITION;

                ClientInvocation::ClientInvocation(spi::ClientContext &client_context,
                                                   std::shared_ptr<protocol::ClientMessage> &&message,
                                                   const std::string &name,
                                                   int partition,
                                                   const std::shared_ptr<connection::Connection> &conn,
                                                   boost::uuids::uuid uuid) :
                        logger_(client_context.get_logger()),
                        lifecycle_service_(client_context.get_lifecycle_service()),
                        client_cluster_service_(client_context.get_client_cluster_service()),
                        invocation_service_(client_context.get_invocation_service()),
                        execution_service_(client_context.get_client_execution_service().shared_from_this()),
                        call_id_sequence_(client_context.get_call_id_sequence()),
                        uuid_(uuid),
                        partition_id_(partition),
                        start_time_(std::chrono::steady_clock::now()),
                        retry_pause_(invocation_service_.get_invocation_retry_pause()),
                        object_name_(name),
                        connection_(conn),
                        invoke_count_(0), urgent_(false), smart_routing_(invocation_service_.is_smart_routing()) {
                    message->set_partition_id(partition_id_);
                    client_message_ = boost::make_shared<std::shared_ptr<protocol::ClientMessage>>(message);
                    set_send_connection(nullptr);
                }

                ClientInvocation::~ClientInvocation() = default;

                boost::future<protocol::ClientMessage> ClientInvocation::invoke() {
                    assert (client_message_.load());
                    // for back pressure
                    call_id_sequence_->next();
                    invoke_on_selection();
                    return invocation_promise_.get_future().then(boost::launch::sync,
                                                               [=](boost::future<protocol::ClientMessage> f) {
                                                                   call_id_sequence_->complete();
                                                                   return f.get();
                                                               });
                }

                boost::future<protocol::ClientMessage> ClientInvocation::invoke_urgent() {
                    assert(client_message_.load());
                    urgent_ = true;
                    // for back pressure
                    call_id_sequence_->force_next();
                    invoke_on_selection();
                    return invocation_promise_.get_future().then(boost::launch::sync,
                                                               [=](boost::future<protocol::ClientMessage> f) {
                                                                   call_id_sequence_->complete();
                                                                   return f.get();
                                                               });
                }

                void ClientInvocation::invoke_on_selection() {
                    try {
                        invoke_count_++;
                        if (!urgent_) {
                            invocation_service_.check_invocation_allowed();
                        }

                        if (is_bind_to_single_connection()) {
                            auto invoked = invocation_service_.invoke_on_connection(shared_from_this(), connection_);
                            if (!invoked) {
                                notify_exception(std::make_exception_ptr(exception::io("", (boost::format(
                                        "Could not invoke on connection %1%") % *connection_).str())));
                            }
                            return;
                        }

                        bool invoked = false;
                        if (smart_routing_) {
                            if (partition_id_ != -1) {
                                invoked = invocation_service_.invoke_on_partition_owner(shared_from_this(), partition_id_);
                            } else if (!uuid_.is_nil()) {
                                invoked = invocation_service_.invoke_on_target(shared_from_this(), uuid_);
                            } else {
                                invoked = invocation_service_.invoke(shared_from_this());
                            }
                            if (!invoked) {
                                invoked = invocation_service_.invoke(shared_from_this());
                            }
                        } else {
                            invoked = invocation_service_.invoke(shared_from_this());
                        }
                        if (!invoked) {
                            notify_exception(std::make_exception_ptr(exception::io("No connection found to invoke")));
                        }
                    } catch (exception::iexception &) {
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
                    client_message_ = boost::make_shared<std::shared_ptr<protocol::ClientMessage>>(copy_message());

                    try {
                        invoke_on_selection();
                    } catch (exception::iexception &e) {
                        set_exception(e, boost::current_exception());
                    } catch (std::exception &) {
                        assert(false);
                    }
                }

                void ClientInvocation::set_exception(const exception::iexception &e, boost::exception_ptr exception_ptr) {
                    try {
                        auto send_conn = *send_connection_.load();
                        if (send_conn) {
                            auto call_id = client_message_.load()->get()->get_correlation_id();
                            boost::asio::post(send_conn->get_socket().get_executor(), [=] () {
                                send_conn->deregister_invocation(call_id);
                            });
                        }
                        invocation_promise_.set_exception(std::move(exception_ptr));
                    } catch (boost::promise_already_satisfied &se) {
                        if (!event_handler_) {
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
                    } catch (exception::iexception &iex) {
                        log_exception(iex);

                        if (!lifecycle_service_.is_running()) {
                            try {
                                std::throw_with_nested(boost::enable_current_exception(
                                        exception::hazelcast_client_not_active(iex.get_source(),
                                                                                     "Client is shutting down")));
                            } catch (exception::iexception &e) {
                                set_exception(e, boost::current_exception());
                            }
                            return;
                        }

                        if (!should_retry(iex)) {
                            set_exception(iex, boost::current_exception());
                            return;
                        }

                        auto timePassed = std::chrono::steady_clock::now() - start_time_;
                        if (timePassed > invocation_service_.get_invocation_timeout()) {
                            HZ_LOG(logger_, finest,
                                boost::str(boost::format("Exception will not be retried because "
                                                         "invocation timed out. %1%") % iex.what())
                            );

                            auto now = std::chrono::steady_clock::now();

                            auto timeoutException = (exception::exception_builder<exception::operation_timeout>(
                                    "ClientInvocation::newoperation_timeout_exception") << *this
                                            << " timed out because exception occurred after client invocation timeout "
                                            << std::chrono::duration_cast<std::chrono::milliseconds>(invocation_service_.get_invocation_timeout()).count()
                                            << "msecs. Last exception:" << iex
                                            << " Current time :" << util::StringUtil::time_to_string(now) << ". "
                                            << "Start time: " << util::StringUtil::time_to_string(start_time_)
                                            << ". Total elapsed time: " <<
                                            std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count()
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
                        } catch (exception::iexception &e) {
                            set_exception(e, boost::current_exception());
                        }
                    } catch (...) {
                        assert(false);
                    }
                }

                void ClientInvocation::erase_invocation() const {
                    if (!this->event_handler_) {
                        auto sent_connection = get_send_connection();
                        if (sent_connection) {
                            auto this_invocation = shared_from_this();
                            boost::asio::post(sent_connection->get_socket().get_executor(), [=] () {
                                sent_connection->invocations.erase(this_invocation->get_client_message()->get_correlation_id());
                            });
                        }
                    }
                }

                bool ClientInvocation::should_retry(exception::iexception &exception) {
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
                        return client_message_.load()->get()->is_retryable() || invocation_service_.is_redo_operation();
                    }
                    return false;
                }

                std::ostream &operator<<(std::ostream &os, const ClientInvocation &invocation) {
                    std::ostringstream target;
                    if (invocation.is_bind_to_single_connection()) {
                        target << "connection " << *invocation.connection_;
                    } else if (invocation.partition_id_ != -1) {
                        target << "partition " << invocation.partition_id_;
                    } else if (!invocation.uuid_.is_nil()) {
                        target << "uuid " << boost::to_string(invocation.uuid_);
                    } else {
                        target << "random";
                    }
                    os << "ClientInvocation{" << "requestMessage = " << *invocation.client_message_.load()->get()
                       << ", objectName = "
                       << invocation.object_name_ << ", target = " << target.str() << ", sendConnection = ";
                    auto sendConnection = invocation.get_send_connection();
                    if (sendConnection) {
                        os << *sendConnection;
                    } else {
                        os << "nullptr";
                    }
                    os << ", backup_acks_expected_ = " << static_cast<int>(invocation.backup_acks_expected_)
                       << ", backup_acks_received = " << invocation.backup_acks_received_;

                    if (invocation.pending_response_) {
                        os << ", pending_response: " << *invocation.pending_response_;
                    }

                    os << '}';

                    return os;
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &client_context,
                                                                           std::shared_ptr<protocol::ClientMessage> &&client_message,
                                                                           const std::string &object_name,
                                                                           int partition_id) {
                    return std::shared_ptr<ClientInvocation>(
                            new ClientInvocation(client_context, std::move(client_message), object_name, partition_id));
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &client_context,
                                                                           std::shared_ptr<protocol::ClientMessage> &&client_message,
                                                                           const std::string &object_name,
                                                                           const std::shared_ptr<connection::Connection> &connection) {
                    return std::shared_ptr<ClientInvocation>(
                            new ClientInvocation(client_context, std::move(client_message), object_name, UNASSIGNED_PARTITION,
                                                 connection));
                }


                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &client_context,
                                                                           std::shared_ptr<protocol::ClientMessage> &&client_message,
                                                                           const std::string &object_name,
                                                                           boost::uuids::uuid uuid) {
                    return std::shared_ptr<ClientInvocation>(
                            new ClientInvocation(client_context, std::move(client_message), object_name, UNASSIGNED_PARTITION,
                                                 nullptr, uuid));
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &client_context,
                                                                           protocol::ClientMessage &client_message,
                                                                           const std::string &object_name,
                                                                           int partition_id) {
                    return create(client_context, std::make_shared<protocol::ClientMessage>(std::move(client_message)),
                                  object_name, partition_id);
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &client_context,
                                                                           protocol::ClientMessage &client_message,
                                                                           const std::string &object_name,
                                                                           const std::shared_ptr<connection::Connection> &connection) {
                    return create(client_context, std::make_shared<protocol::ClientMessage>(std::move(client_message)),
                                  object_name, connection);
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &client_context,
                                                                           protocol::ClientMessage &client_message,
                                                                           const std::string &object_name,
                                                                           boost::uuids::uuid uuid) {
                    return create(client_context, std::make_shared<protocol::ClientMessage>(std::move(client_message)),
                                  object_name, uuid);
                }

                std::shared_ptr<connection::Connection> ClientInvocation::get_send_connection() const {
                    return *send_connection_.load();
                }

                std::shared_ptr<connection::Connection> ClientInvocation::get_send_connection_or_wait() const {
                    while (!(*send_connection_.load()) && lifecycle_service_.is_running()) {
                        std::this_thread::yield();
                    }
                    if (!lifecycle_service_.is_running()) {
                        BOOST_THROW_EXCEPTION(exception::illegal_argument("Client is being shut down!"));
                    }
                    return *send_connection_.load();
                }

                void
                ClientInvocation::set_send_connection(const std::shared_ptr<connection::Connection> &conn) {
                    send_connection_.store(boost::make_shared<std::shared_ptr<connection::Connection>>(conn));
                }

                void ClientInvocation::notify(const std::shared_ptr<protocol::ClientMessage> &msg) {
                    if (!msg) {
                        BOOST_THROW_EXCEPTION(exception::illegal_argument("response can't be null"));
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
                        this->invocation_promise_.set_value(*msg);
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
                    return *client_message_.load();
                }

                const std::shared_ptr<EventHandler<protocol::ClientMessage> > &
                ClientInvocation::get_event_handler() const {
                    return event_handler_;
                }

                void ClientInvocation::set_event_handler(
                        const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                    ClientInvocation::event_handler_ = handler;
                }

                void ClientInvocation::execute() {
                    auto this_invocation = shared_from_this();
                    auto command = [=]() {
                        this_invocation->run();
                    };

                    // first we force a new invocation slot because we are going to return our old invocation slot immediately after
                    // It is important that we first 'force' taking a new slot; otherwise it could be that a sneaky invocation gets
                    // through that takes our slot!
                    int64_t callId = call_id_sequence_->force_next();
                    client_message_.load()->get()->set_correlation_id(callId);

                    //we release the old slot
                    call_id_sequence_->complete();

                    if (invoke_count_ < MAX_FAST_INVOCATION_COUNT) {
                        // fast retry for the first few invocations
                        execution_service_->execute(command);
                    } else {
                        // progressive retry delay
                        int64_t delayMillis = util::min<int64_t>(static_cast<int64_t>(1) << (invoke_count_ - MAX_FAST_INVOCATION_COUNT),
                                                                 std::chrono::duration_cast<std::chrono::milliseconds>(retry_pause_).count());
                        retry_timer_ = execution_service_->schedule(command, std::chrono::milliseconds(delayMillis));
                    }
                }

                const std::string ClientInvocation::get_name() const {
                    return "ClientInvocation";
                }

                std::shared_ptr<protocol::ClientMessage> ClientInvocation::copy_message() {
                    return std::make_shared<protocol::ClientMessage>(**client_message_.load());
                }

                boost::promise<protocol::ClientMessage> &ClientInvocation::get_promise() {
                    return invocation_promise_;
                }

                void ClientInvocation::log_exception(exception::iexception &e) {
                    HZ_LOG(logger_, finest,
                        boost::str(boost::format("Invocation got an exception %1%, invoke count : %2%, "
                                                 "exception : %3%")
                                                 % *this % invoke_count_.load() % e)
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
                ClientInvocation::detect_and_handle_backup_timeout(const std::chrono::milliseconds &backup_timeout) {
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
                    if (pending_response_received_time_ + backup_timeout >= std::chrono::steady_clock::now()) {
                        return;
                    }

                    if (invocation_service_.fail_on_indeterminate_state()) {
                        auto exception = boost::enable_current_exception((exception::exception_builder<exception::indeterminate_operation_state>(
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
                    client_config &clientConfig = client_.get_client_config();
                    bool smartRouting = clientConfig.get_network_config().is_smart_routing();

                    while (client_.get_lifecycle_service().is_running()) {
                        try {
                            auto connection = client_.get_connection_manager().get_random_connection();
                            if (!connection) {
                                throw_exception(smartRouting);
                            }
                            return connection;
                        } catch (exception::hazelcast_client_offline &) {
                            throw;
                        } catch (exception::iexception &) {
                            if (std::chrono::steady_clock::now() - startTime > invocationTimeout) {
                                std::rethrow_exception(
                                        new_operation_timeout_exception(std::current_exception(), invocationTimeout,
                                                                        startTime));
                            }
                        }
                        std::this_thread::sleep_for(invocationService.get_invocation_retry_pause());
                    }
                    BOOST_THROW_EXCEPTION(
                            exception::hazelcast_client_not_active("ClientTransactionManagerServiceImpl::connect",
                                                                   "Client is stopped."));
                }

                std::exception_ptr
                ClientTransactionManagerServiceImpl::new_operation_timeout_exception(std::exception_ptr cause,
                                                                                  std::chrono::milliseconds invocation_timeout,
                                                                                  std::chrono::steady_clock::time_point start_time) {
                    std::ostringstream sb;
                    auto now = std::chrono::steady_clock::now();
                    sb
                            << "Creating transaction context timed out because exception occurred after client invocation timeout "
                            << std::chrono::duration_cast<std::chrono::milliseconds>(invocation_timeout).count() << " ms. " << "Current time: "
                            << util::StringUtil::time_to_string(std::chrono::steady_clock::now()) << ". " << "Start time: "
                            << util::StringUtil::time_to_string(start_time) << ". Total elapsed time: "
                            << std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() << " ms. ";
                    try {
                        std::rethrow_exception(cause);
                    } catch (...) {
                        try {
                            std::throw_with_nested(boost::enable_current_exception(exception::operation_timeout(
                                    "ClientTransactionManagerServiceImpl::newoperation_timeout_exception", sb.str())));
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
                    if (reconnect_mode == config::client_connection_strategy_config::reconnect_mode::ASYNC) {
                        BOOST_THROW_EXCEPTION(exception::hazelcast_client_offline(
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
                        BOOST_THROW_EXCEPTION(exception::illegal_state(
                                                      "ClientTransactionManagerServiceImpl::throw_exception", msg.str()));
                    }
                    BOOST_THROW_EXCEPTION(exception::illegal_state(
                                                  "ClientTransactionManagerServiceImpl::throw_exception",
                                                  "No active connection is found"));
                }

                AwsAddressProvider::AwsAddressProvider(config::client_aws_config &aws_config, int aws_member_port,
                                                       logger &lg) : aws_member_port_(
                        util::IOUtil::to_string<int>(aws_member_port)), logger_(lg), aws_client_(aws_config, lg) {
                }

                std::vector<address> AwsAddressProvider::load_addresses() {
                    update_lookup_table();
                    std::unordered_map<std::string, std::string> lookupTable = get_lookup_table();
                    std::vector<address> addresses;

                    typedef std::unordered_map<std::string, std::string> LookupTable;
                    for (const LookupTable::value_type &privateAddress : lookupTable) {
                        std::vector<address> possibleAddresses = util::AddressHelper::get_socket_addresses(
                                privateAddress.first + ":" + aws_member_port_, logger_);
                        addresses.insert(addresses.begin(), possibleAddresses.begin(),
                                         possibleAddresses.end());
                    }
                    return addresses;
                }

                void AwsAddressProvider::update_lookup_table() {
                    try {
                        private_to_public_ = aws_client_.get_addresses();
                    } catch (exception::iexception &e) {
                        HZ_LOG(logger_, warning,
                            boost::str(boost::format("Aws addresses failed to load: %1%") % e.get_message())
                        );
                    }
                }

                std::unordered_map<std::string, std::string> AwsAddressProvider::get_lookup_table() {
                    return private_to_public_;
                }

                AwsAddressProvider::~AwsAddressProvider() = default;

                address DefaultAddressTranslator::translate(const address &address) {
                    return address;
                }

                void DefaultAddressTranslator::refresh() {
                }


                ClientPartitionServiceImpl::ClientPartitionServiceImpl(ClientContext &client)
                        : client_(client), logger_(client.get_logger()), partition_count_(0),
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

                boost::uuids::uuid ClientPartitionServiceImpl::get_partition_owner(int32_t partition_id) {
                    auto table_ptr = partition_table_.load();
                    auto it = table_ptr->partitions.find(partition_id);
                    if (it != table_ptr->partitions.end()) {
                        return it->second;
                    }
                    return boost::uuids::nil_uuid();
                }

                int32_t ClientPartitionServiceImpl::get_partition_id(const serialization::pimpl::data &key) {
                    int32_t pc = get_partition_count();
                    if (pc <= 0) {
                        return 0;
                    }
                    int hash = key.get_partition_hash();
                    return util::HashUtil::hash_to_index(hash, pc);
                }

                int32_t ClientPartitionServiceImpl::get_partition_count() {
                    return partition_count_.load();
                }

                std::shared_ptr<client::impl::Partition> ClientPartitionServiceImpl::get_partition(int partition_id) {
                    return std::shared_ptr<client::impl::Partition>(new PartitionImpl(partition_id, client_, *this));
                }

                bool ClientPartitionServiceImpl::check_and_set_partition_count(int32_t new_partition_count) {
                    int32_t expected = 0;
                    if (partition_count_.compare_exchange_strong(expected, new_partition_count)) {
                        return true;
                    }
                    return partition_count_.load() == new_partition_count;
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
                    return partition_id_;
                }

                boost::optional<member> ClientPartitionServiceImpl::PartitionImpl::get_owner() const {
                    auto owner = partition_service_.get_partition_owner(partition_id_);
                    if (!owner.is_nil()) {
                        return client_.get_client_cluster_service().get_member(owner);
                    }
                    return boost::none;
                }

                ClientPartitionServiceImpl::PartitionImpl::PartitionImpl(int partition_id, ClientContext &client,
                                                                         ClientPartitionServiceImpl &partition_service)
                        : partition_id_(partition_id), client_(client), partition_service_(partition_service) {
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
                    AbstractCallIdSequence::AbstractCallIdSequence(int32_t max_concurrent_invocations) {
                        std::ostringstream out;
                        out << "maxConcurrentInvocations should be a positive number. maxConcurrentInvocations="
                            << max_concurrent_invocations;
                        this->max_concurrent_invocations_ = util::Preconditions::check_positive(max_concurrent_invocations,
                                                                                            out.str());

                        for (size_t i = 0; i < longs_.size(); ++i) {
                            longs_[i] = 0;
                        }
                    }

                    AbstractCallIdSequence::~AbstractCallIdSequence() = default;

                    int32_t AbstractCallIdSequence::get_max_concurrent_invocations() const {
                        return max_concurrent_invocations_;
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
                        return longs_[INDEX_HEAD] - longs_[INDEX_TAIL] < max_concurrent_invocations_;
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

                    CallIdSequenceWithBackpressure::CallIdSequenceWithBackpressure(int32_t max_concurrent_invocations,
                                                                                   int64_t backoff_timeout_ms)
                            : AbstractCallIdSequence(max_concurrent_invocations) {
                        std::ostringstream out;
                        out << "backoffTimeoutMs should be a positive number. backoffTimeoutMs=" << backoff_timeout_ms;
                        util::Preconditions::check_positive(backoff_timeout_ms, out.str());

                        backoff_timeout_nanos_ = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                std::chrono::milliseconds(backoff_timeout_ms)).count();
                    }

                    void CallIdSequenceWithBackpressure::handle_no_space_left() {
                        auto start = std::chrono::steady_clock::now();
                        for (int64_t idleCount = 0;; idleCount++) {
                            int64_t elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                    std::chrono::steady_clock::now() - start).count();
                            if (elapsedNanos > backoff_timeout_nanos_) {
                                throw (exception::exception_builder<exception::hazelcast_overload>(
                                        "CallIdSequenceWithBackpressure::handleNoSpaceLeft")
                                        << "Timed out trying to acquire another call ID."
                                        << " maxConcurrentInvocations = " << get_max_concurrent_invocations()
                                        << ", backoffTimeout = "
                                        << std::chrono::microseconds(backoff_timeout_nanos_ / 1000).count()
                                        << " msecs, elapsed:"
                                        << std::chrono::microseconds(elapsedNanos / 1000).count() << " msecs").build();
                            }
                            IDLER->idle(idleCount);
                            if (has_space()) {
                                return;
                            }

                        }
                    }

                    FailFastCallIdSequence::FailFastCallIdSequence(int32_t max_concurrent_invocations)
                            : AbstractCallIdSequence(max_concurrent_invocations) {}

                    void FailFastCallIdSequence::handle_no_space_left() {
                        throw (exception::exception_builder<exception::hazelcast_overload>(
                                "FailFastCallIdSequence::handleNoSpaceLeft")
                                << "Maximum invocation count is reached. maxConcurrentInvocations = "
                                << get_max_concurrent_invocations()).build();

                    }

                    std::unique_ptr<CallIdSequence> CallIdFactory::new_call_id_sequence(bool is_back_pressure_enabled,
                                                                                     int32_t max_allowed_concurrent_invocations,
                                                                                     int64_t backoff_timeout_ms) {
                        if (!is_back_pressure_enabled) {
                            return std::unique_ptr<CallIdSequence>(new CallIdSequenceWithoutBackpressure());
                        } else if (backoff_timeout_ms <= 0) {
                            return std::unique_ptr<CallIdSequence>(
                                    new FailFastCallIdSequence(max_allowed_concurrent_invocations));
                        } else {
                            return std::unique_ptr<CallIdSequence>(
                                    new CallIdSequenceWithBackpressure(max_allowed_concurrent_invocations,
                                                                       backoff_timeout_ms));
                        }
                    }
                }

                namespace listener {
                    listener_service_impl::listener_service_impl(ClientContext &client_context,
                                                                 int32_t event_thread_count)
                            : client_context_(client_context),
                              serialization_service_(client_context.get_serialization_service()),
                              logger_(client_context.get_logger()),
                              client_connection_manager_(client_context.get_connection_manager()),
                              number_of_event_threads_(event_thread_count),
                              smart_(client_context.get_client_config().get_network_config().is_smart_routing()) {
                        auto &invocationService = client_context.get_invocation_service();
                        invocation_timeout_ = invocationService.get_invocation_timeout();
                        invocation_retry_pause_ = invocationService.get_invocation_retry_pause();
                    }

                    bool listener_service_impl::registers_local_only() const {
                        return smart_;
                    }

                    boost::future<boost::uuids::uuid>
                    listener_service_impl::register_listener(
                            std::shared_ptr<ListenerMessageCodec> listener_message_codec,
                            std::shared_ptr<client::impl::BaseEventHandler> handler) {
                        auto task = boost::packaged_task<boost::uuids::uuid()>([=]() {
                                    return register_listener_internal(listener_message_codec, handler);
                                });
                        auto f = task.get_future();
                        boost::asio::post(registration_executor_->get_executor(), std::move(task));
                        return f;
                    }

                    boost::future<bool> listener_service_impl::deregister_listener(boost::uuids::uuid registration_id) {
                        util::Preconditions::check_not_nill(registration_id, "Nil userRegistrationId is not allowed!");

                        boost::packaged_task<bool()> task([=]() {
                            return deregister_listener_internal(registration_id);
                        });
                        auto f = task.get_future();
                        boost::asio::post(registration_executor_->get_executor(), std::move(task));
                        return f;
                    }

                    void listener_service_impl::connection_added(
                            const std::shared_ptr<connection::Connection> connection) {
                        boost::asio::post(registration_executor_->get_executor(), [=]() { connection_added_internal(connection); });
                    }

                    void listener_service_impl::connection_removed(
                            const std::shared_ptr<connection::Connection> connection) {
                        boost::asio::post(registration_executor_->get_executor(), [=]() { connection_removed_internal(connection); });
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
                                boost::asio::post(event_executor_->get_executor(), [=]() { process_event_message(invocation, response); });
                                return;
                            }

                            // process on certain thread which is same for the partition id
                            boost::asio::post(event_strands_[partitionId % event_strands_.size()],
                                              [=]() { process_event_message(invocation, response); });

                        } catch (const std::exception &e) {
                            if (client_context_.get_lifecycle_service().is_running()) {
                                HZ_LOG(logger_, warning,
                                    boost::str(boost::format("Delivery of event message to event handler failed. %1%, %2%, %3%")
                                                             % e.what() % *response % *invocation)
                                );
                            }
                        }
                    }

                    void listener_service_impl::shutdown() {
                        ClientExecutionServiceImpl::shutdown_thread_pool(event_executor_.get());
                        ClientExecutionServiceImpl::shutdown_thread_pool(registration_executor_.get());
                    }

                    void listener_service_impl::start() {
                        event_executor_.reset(new hazelcast::util::hz_thread_pool(number_of_event_threads_));
                        registration_executor_.reset(new hazelcast::util::hz_thread_pool(1));

                        for (int i = 0; i < number_of_event_threads_; ++i) {
                            event_strands_.emplace_back(event_executor_->get_executor());
                        }

                        client_connection_manager_.add_connection_listener(shared_from_this());
                    }

                    boost::uuids::uuid listener_service_impl::register_listener_internal(
                            std::shared_ptr<ListenerMessageCodec> listener_message_codec,
                            std::shared_ptr<client::impl::BaseEventHandler> handler) {
                        auto user_registration_id = client_context_.random_uuid();

                        std::shared_ptr<listener_registration> registration(new listener_registration{listener_message_codec, handler});
                        registrations_.put(user_registration_id, registration);
                        for (auto const &connection : client_connection_manager_.get_active_connections()) {
                            try {
                                invoke(registration, connection);
                            } catch (exception::iexception &e) {
                                if (connection->is_alive()) {
                                    deregister_listener_internal(user_registration_id);
                                    BOOST_THROW_EXCEPTION((exception::exception_builder<exception::hazelcast_>(
                                            "ClientListenerService::RegisterListenerTask::call")
                                            << "Listener can not be added " << e).build());
                                }
                            }
                        }
                        return user_registration_id;
                    }

                    bool
                    listener_service_impl::deregister_listener_internal(boost::uuids::uuid user_registration_id) {
                        auto listenerRegistration = registrations_.get(user_registration_id);
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
                                auto invocation = ClientInvocation::create(client_context_,request, "",
                                                                           subscriber);
                                invocation->invoke().get();

                                remove_event_handler(registration->call_id, subscriber);

                                it = listener_registrations.erase(it);
                            } catch (exception::iexception &e) {
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
                                                                 % user_registration_id
                                                                 % subscriber->get_remote_address() % e)
                                    );
                                }
                            }
                        }
                        if (successful) {
                            registrations_.remove(user_registration_id);
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
                        } catch (exception::iexception &e) {
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

                        auto invocation = ClientInvocation::create(client_context_,
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
                            if (client_context_.get_lifecycle_service().is_running()) {
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
                            if (client_context_.get_lifecycle_service().is_running()) {
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

                    cluster_view_listener::cluster_view_listener(ClientContext &client_context) : client_context_(
                            client_context), listener_added_connection_(nullptr) {}

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
                                                                             const std::vector<member> &member_infos) {
                        view_listener.client_context_.get_client_cluster_service().handle_event(version, member_infos);
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
                            cluster_view_listener &view_listener) : connection(connection),
                                                                   view_listener(view_listener) {}
                }

                protocol::ClientMessage
                ClientInvocationServiceImpl::BackupListenerMessageCodec::encode_add_request(bool local_only) const {
                    return protocol::codec::client_localbackuplistener_encode();
                }

                protocol::ClientMessage ClientInvocationServiceImpl::BackupListenerMessageCodec::encode_remove_request(
                        boost::uuids::uuid real_registration_id) const {
                    assert(0);
                    return protocol::ClientMessage(0);
                }

                void ClientInvocationServiceImpl::noop_backup_event_handler::handle_backup(
                        int64_t source_invocation_correlation_id) {
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



