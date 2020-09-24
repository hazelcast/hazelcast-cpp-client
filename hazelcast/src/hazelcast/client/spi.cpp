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
        const std::unordered_set<Member> &InitialMembershipEvent::getMembers() const {
            return members_;
        }

        Cluster &InitialMembershipEvent::getCluster() {
            return cluster_;
        }

        InitialMembershipEvent::InitialMembershipEvent(Cluster &cluster, std::unordered_set<Member> members) : cluster_(
                cluster), members_(std::move(members)) {
        }

        LifecycleEvent::LifecycleEvent(LifecycleState state)
                : state_(state) {
        }

        LifecycleEvent::LifecycleState LifecycleEvent::getState() const {
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
                        p.second.get()->onShutdown();
                    } catch (std::exception &se) {
                        auto &logger = client_.getLogger();
                        if (logger.isFinestEnabled()) {
                            logger.finest("Proxy was not created, hence onShutdown can be called. Exception:",
                                          se.what());
                        }
                    }
                }
                proxies_.clear();
            }

            void ProxyManager::initialize(const std::shared_ptr<ClientProxy> &clientProxy) {
                auto clientMessage = protocol::codec::client_createproxy_encode(clientProxy->getName(),
                        clientProxy->getServiceName());
                spi::impl::ClientInvocation::create(client_, clientMessage, clientProxy->getServiceName())->invoke().get();
                clientProxy->onInitialize();
            }

            boost::future<void> ProxyManager::destroyProxy(ClientProxy &proxy) {
                DefaultObjectNamespace objectNamespace(proxy.getServiceName(), proxy.getName());
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
                            proxy.destroyLocally();
                            return proxy.destroyRemotely();
                        } catch (exception::IException &) {
                            proxy.destroyRemotely();
                            throw;
                        }
                    }
                    if (&proxy != registeredProxy.get()) {
                        // The given proxy is stale and was already destroyed, but the caller
                        // may have allocated local resources in the context of this stale proxy
                        // instance after it was destroyed, so we have to cleanup it locally one
                        // more time to make sure there are no leaking local resources.
                        proxy.destroyLocally();
                    }
                } catch (...) {
                    if (&proxy != registeredProxy.get()) {
                        // The given proxy is stale and was already destroyed, but the caller
                        // may have allocated local resources in the context of this stale proxy
                        // instance after it was destroyed, so we have to cleanup it locally one
                        // more time to make sure there are no leaking local resources.
                        proxy.destroyLocally();
                    }
                    throw;
                }
                return boost::make_ready_future();
            }

            ClientContext::ClientContext(const client::HazelcastClient &hazelcastClient) : hazelcast_client_(
                    *hazelcastClient.client_impl_) {
            }

            ClientContext::ClientContext(client::impl::HazelcastClientInstanceImpl &hazelcastClient)
                    : hazelcast_client_(hazelcastClient) {
            }

            serialization::pimpl::SerializationService &ClientContext::getSerializationService() {
                return hazelcast_client_.serialization_service_;
            }

            impl::ClientClusterServiceImpl & ClientContext::getClientClusterService() {
                return hazelcast_client_.cluster_service_;
            }

            impl::ClientInvocationServiceImpl &ClientContext::getInvocationService() {
                return *hazelcast_client_.invocation_service_;
            }

            ClientConfig &ClientContext::getClientConfig() {
                return hazelcast_client_.client_config_;
            }

            impl::ClientPartitionServiceImpl & ClientContext::getPartitionService() {
                return *hazelcast_client_.partition_service_;
            }

            LifecycleService &ClientContext::getLifecycleService() {
                return hazelcast_client_.lifecycle_service_;
            }

            spi::impl::listener::listener_service_impl &ClientContext::getClientListenerService() {
                return *hazelcast_client_.listener_service_;
            }

            connection::ClientConnectionManagerImpl &ClientContext::getConnectionManager() {
                return *hazelcast_client_.connection_manager_;
            }

            internal::nearcache::NearCacheManager &ClientContext::getNearCacheManager() {
                return *hazelcast_client_.near_cache_manager_;
            }

            ClientProperties &ClientContext::getClientProperties() {
                return hazelcast_client_.client_properties_;
            }

            Cluster &ClientContext::getCluster() {
                return hazelcast_client_.cluster_;
            }

            std::shared_ptr<impl::sequence::CallIdSequence> &ClientContext::getCallIdSequence() const {
                return hazelcast_client_.call_id_sequence_;
            }

            const protocol::ClientExceptionFactory &ClientContext::getClientExceptionFactory() const {
                return hazelcast_client_.getExceptionFactory();
            }

            const std::string &ClientContext::getName() const {
                return hazelcast_client_.getName();
            }

            impl::ClientExecutionServiceImpl &ClientContext::getClientExecutionService() const {
                return *hazelcast_client_.execution_service_;
            }

            const std::shared_ptr<client::impl::ClientLockReferenceIdGenerator> &
            ClientContext::getLockReferenceIdGenerator() {
                return hazelcast_client_.getLockReferenceIdGenerator();
            }

            std::shared_ptr<client::impl::HazelcastClientInstanceImpl>
            ClientContext::getHazelcastClientImplementation() {
                return hazelcast_client_.shared_from_this();
            }

            spi::ProxyManager &ClientContext::getProxyManager() {
                return hazelcast_client_.getProxyManager();
            }

            util::ILogger &ClientContext::getLogger() {
                return *hazelcast_client_.logger_;
            }

            client::impl::statistics::Statistics &ClientContext::getClientstatistics() {
                return *hazelcast_client_.statistics_;
            }

            spi::impl::listener::cluster_view_listener &ClientContext::get_cluster_view_listener() {
                return *hazelcast_client_.cluster_listener_;
            }

            boost::uuids::uuid ClientContext::random_uuid() {
                return hazelcast_client_.random_uuid();
            }

            LifecycleService::LifecycleService(ClientContext &clientContext,
                                               const std::vector<LifecycleListener> &listeners,
                                               LoadBalancer *const loadBalancer, Cluster &cluster) : 
                    client_context_(clientContext), listeners_(), load_balancer_(loadBalancer),
                    cluster_(cluster), shutdown_completed_latch_(1) {
                for (const auto &listener: listeners) {
                    addListener(LifecycleListener(listener));
                }
            }

            bool LifecycleService::start() {
                bool expected = false;
                if (!active_.compare_exchange_strong(expected, true)) {
                    return false;
                }

                fireLifecycleEvent(LifecycleEvent::STARTED);

                client_context_.getClientExecutionService().start();

                client_context_.getClientListenerService().start();

                client_context_.getInvocationService().start();

                client_context_.getClientClusterService().start();

                client_context_.get_cluster_view_listener().start();

                if (!client_context_.getConnectionManager().start()) {
                    return false;
                }

                auto &connectionStrategyConfig = client_context_.getClientConfig().getConnectionStrategyConfig();
                if (!connectionStrategyConfig.isAsyncStart()) {
                    // The client needs to open connections to all members before any services requiring internal listeners start
                    wait_for_initial_membership_event();
                    client_context_.getConnectionManager().connect_to_all_cluster_members();
                }

                load_balancer_->init(cluster_);

                client_context_.getClientstatistics().start();

                return true;
            }

            void LifecycleService::shutdown() {
                bool expected = true;
                if (!active_.compare_exchange_strong(expected, false)) {
                    shutdown_completed_latch_.wait();
                    return;
                }
                try {
                    fireLifecycleEvent(LifecycleEvent::SHUTTING_DOWN);
                    client_context_.getClientstatistics().shutdown();
                    client_context_.getProxyManager().destroy();
                    client_context_.getConnectionManager().shutdown();
                    client_context_.getClientClusterService().shutdown();
                    client_context_.getInvocationService().shutdown();
                    client_context_.getClientListenerService().shutdown();
                    client_context_.getNearCacheManager().destroyAllNearCaches();
                    fireLifecycleEvent(LifecycleEvent::SHUTDOWN);
                    client_context_.getClientExecutionService().shutdown();
                    client_context_.getSerializationService().dispose();
                    shutdown_completed_latch_.count_down();
                } catch (std::exception &e) {
                    client_context_.getLogger().info("An exception occured during LifecycleService shutdown. ", e.what());
                    shutdown_completed_latch_.count_down();
                }
            }

            boost::uuids::uuid LifecycleService::addListener(LifecycleListener &&lifecycleListener) {
                std::lock_guard<std::mutex> lg(listener_lock_);
                const auto id = uuid_generator_();
                listeners_.emplace(id, std::move(lifecycleListener));
                return id;
            }

            bool LifecycleService::removeListener(const boost::uuids::uuid &registrationId) {
                std::lock_guard<std::mutex> lg(listener_lock_);
                return listeners_.erase(registrationId) == 1;
            }

            void LifecycleService::fireLifecycleEvent(const LifecycleEvent &lifecycleEvent) {
                std::lock_guard<std::mutex> lg(listener_lock_);
                util::ILogger &logger = client_context_.getLogger();

                std::function<void(LifecycleListener &)> fire_one;

                switch (lifecycleEvent.getState()) {
                    case LifecycleEvent::STARTING : {
                        // convert the date string from "2016-04-20" to 20160420
                        std::string date(HAZELCAST_STRINGIZE(HAZELCAST_GIT_COMMIT_DATE));
                        util::gitDateToHazelcastLogDate(date);
                        std::string commitId(HAZELCAST_STRINGIZE(HAZELCAST_GIT_COMMIT_ID));
                        commitId.erase(std::remove(commitId.begin(), commitId.end(), '"'), commitId.end());
                        char msg[100];
                        util::hz_snprintf(msg, 100, "(%s:%s) LifecycleService::LifecycleEvent STARTING", date.c_str(),
                                          commitId.c_str());
                        logger.info(msg);

                        fire_one = [](LifecycleListener &listener) {
                            listener.starting_();
                        };
                        break;
                    }
                    case LifecycleEvent::STARTED : {
                        logger.info("LifecycleService::LifecycleEvent STARTED");

                        fire_one = [](LifecycleListener &listener) {
                            listener.started_();
                        };
                        break;
                    }
                    case LifecycleEvent::SHUTTING_DOWN : {
                        logger.info("LifecycleService::LifecycleEvent SHUTTING_DOWN");

                        fire_one = [](LifecycleListener &listener) {
                            listener.shutting_down_();
                        };
                        break;
                    }
                    case LifecycleEvent::SHUTDOWN : {
                        logger.info("LifecycleService::LifecycleEvent SHUTDOWN");

                        fire_one = [](LifecycleListener &listener) {
                            listener.shutdown_();
                        };
                        break;
                    }
                    case LifecycleEvent::CLIENT_CONNECTED : {
                        logger.info("LifecycleService::LifecycleEvent CLIENT_CONNECTED");

                        fire_one = [](LifecycleListener &listener) {
                            listener.connected_();
                        };
                        break;
                    }
                    case LifecycleEvent::CLIENT_DISCONNECTED : {
                        logger.info("LifecycleService::LifecycleEvent CLIENT_DISCONNECTED");

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

            bool LifecycleService::isRunning() {
                return active_;
            }

            LifecycleService::~LifecycleService() {
                if (active_) {
                    shutdown();
                }
            }

            void LifecycleService::wait_for_initial_membership_event() const {
                client_context_.getClientClusterService().wait_initial_member_list_fetched();
            }

            DefaultObjectNamespace::DefaultObjectNamespace(const std::string &service, const std::string &object)
                    : service_name_(service), object_name_(object) {

            }

            const std::string &DefaultObjectNamespace::getServiceName() const {
                return service_name_;
            }

            const std::string &DefaultObjectNamespace::getObjectName() const {
                return object_name_;
            }

            bool DefaultObjectNamespace::operator==(const DefaultObjectNamespace &rhs) const {
                return service_name_ == rhs.service_name_ && object_name_ == rhs.object_name_;
            }

            ClientProxy::ClientProxy(const std::string &name, const std::string &serviceName, ClientContext &context)
                    : name(name), service_name_(serviceName), context_(context) {}

            ClientProxy::~ClientProxy() = default;

            const std::string &ClientProxy::getName() const {
                return name;
            }

            const std::string &ClientProxy::getServiceName() const {
                return service_name_;
            }

            ClientContext &ClientProxy::getContext() {
                return context_;
            }

            void ClientProxy::onDestroy() {
            }

            boost::future<void> ClientProxy::destroy() {
                return getContext().getProxyManager().destroyProxy(*this);
            }

            void ClientProxy::destroyLocally() {
                if (preDestroy()) {
                    try {
                        onDestroy();
                        postDestroy();
                    } catch (exception::IException &) {
                        postDestroy();
                        throw;
                    }
                }
            }

            bool ClientProxy::preDestroy() {
                return true;
            }

            void ClientProxy::postDestroy() {
            }

            void ClientProxy::onInitialize() {
            }

            void ClientProxy::onShutdown() {
            }

            serialization::pimpl::SerializationService &ClientProxy::getSerializationService() {
                return context_.getSerializationService();
            }

            boost::future<void> ClientProxy::destroyRemotely() {
                auto clientMessage = protocol::codec::client_destroyproxy_encode(
                        getName(), getServiceName());
                return spi::impl::ClientInvocation::create(getContext(), std::make_shared<protocol::ClientMessage>(std::move(clientMessage)), getName())->invoke().then(
                        boost::launch::deferred, [](boost::future<protocol::ClientMessage> f) { f.get(); });
            }

            boost::future<boost::uuids::uuid>
            ClientProxy::registerListener(std::shared_ptr<impl::ListenerMessageCodec> listenerMessageCodec,
                                          std::shared_ptr<client::impl::BaseEventHandler> handler) {
                handler->setLogger(&getContext().getLogger());
                return getContext().getClientListenerService().registerListener(listenerMessageCodec,
                                                                                handler);
            }

            boost::future<bool> ClientProxy::deregisterListener(boost::uuids::uuid registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            namespace impl {
                boost::uuids::uuid
                ListenerMessageCodec::decodeAddResponse(protocol::ClientMessage &msg) const {
                    return msg.get_first_uuid();
                }

                bool ListenerMessageCodec::decodeRemoveResponse(protocol::ClientMessage &msg) const {
                    return msg.get_first_fixed_sized_field<bool>();
                }

                ClientInvocationServiceImpl::ClientInvocationServiceImpl(ClientContext &client)
                        : client_(client), invocation_logger_(client.getLogger()),
                          invocation_timeout_(std::chrono::seconds(client.getClientProperties().getInteger(
                                  client.getClientProperties().getInvocationTimeoutSeconds()))),
                          invocation_retry_pause_(std::chrono::milliseconds(client.getClientProperties().getLong(
                                  client.getClientProperties().getInvocationRetryPauseMillis()))),
                          response_thread_(invocation_logger_, *this, client),
                          smart_routing_(client.getClientConfig().getNetworkConfig().isSmartRouting()) {
                }

                void ClientInvocationServiceImpl::start() {
                    response_thread_.start();
                }

                void ClientInvocationServiceImpl::shutdown() {
                    is_shutdown_.store(true);

                    response_thread_.shutdown();
                }

                std::chrono::steady_clock::duration ClientInvocationServiceImpl::getInvocationTimeout() const {
                    return invocation_timeout_;
                }

                std::chrono::steady_clock::duration ClientInvocationServiceImpl::getInvocationRetryPause() const {
                    return invocation_retry_pause_;
                }

                bool ClientInvocationServiceImpl::isRedoOperation() {
                    return client_.getClientConfig().isRedoOperation();
                }

                void
                ClientInvocationServiceImpl::handleClientMessage(const std::shared_ptr<ClientInvocation> &invocation,
                                                                 const std::shared_ptr<ClientMessage> &response) {
                    response_thread_.process(invocation, response);
                }

                bool ClientInvocationServiceImpl::send(const std::shared_ptr<impl::ClientInvocation>& invocation,
                                                           const std::shared_ptr<connection::Connection>& connection) {
                    if (is_shutdown_) {
                        BOOST_THROW_EXCEPTION(
                                exception::HazelcastClientNotActiveException("ClientInvocationServiceImpl::send",
                                                                             "Client is shut down"));
                    }

                    writeToConnection(*connection, invocation);
                    invocation->setSendConnection(connection);
                    return true;
                }

                void ClientInvocationServiceImpl::writeToConnection(connection::Connection &connection,
                                                                        const std::shared_ptr<ClientInvocation> &clientInvocation) {
                    auto clientMessage = clientInvocation->getClientMessage();
                    connection.write(clientInvocation);
                }

                void ClientInvocationServiceImpl::check_invocation_allowed() {
                    client_.getConnectionManager().check_invocation_allowed();
                }

                bool ClientInvocationServiceImpl::invoke(std::shared_ptr<ClientInvocation> invocation) {
                    auto connection = client_.getConnectionManager().get_random_connection();
                    if (!connection) {
                        if (invocation_logger_.isFinestEnabled()) {
                            invocation_logger_.finest("No connection found to invoke");
                        }
                        return false;
                    }
                    return send(invocation, connection);
                }

                ClientInvocationServiceImpl::ResponseProcessor::ResponseProcessor(util::ILogger &invocationLogger,
                                                                                  ClientInvocationServiceImpl &invocationService,
                                                                                  ClientContext &clientContext)
                        : invocation_logger_(invocationLogger), client_(clientContext) {
                }

                void ClientInvocationServiceImpl::ResponseProcessor::processInternal(
                        const std::shared_ptr<ClientInvocation> &invocation,
                        const std::shared_ptr<protocol::ClientMessage> &response) {
                    try {
                        if (protocol::codec::ErrorCodec::EXCEPTION_MESSAGE_TYPE == response->getMessageType()) {
                            auto error_holder = protocol::codec::ErrorCodec::decode(*response);
                            invocation->notifyException(client_.getClientExceptionFactory().create_exception(error_holder));
                        } else {
                            invocation->notify(response);
                        }
                    } catch (std::exception &e) {
                        invocation_logger_.severe("Failed to process response for ", *invocation, ". ", e.what());
                    }
                }

                void ClientInvocationServiceImpl::ResponseProcessor::shutdown() {
                    ClientExecutionServiceImpl::shutdownThreadPool(pool_.get());
                }

                void ClientInvocationServiceImpl::ResponseProcessor::start() {
                    ClientProperties &clientProperties = client_.getClientProperties();
                    auto threadCount = clientProperties.getInteger(clientProperties.getResponseExecutorThreadCount());
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
                        processInternal(invocation, response);
                        return;
                    }

                    boost::asio::post(pool_->get_executor(), [=] { processInternal(invocation, response); });
                }

                DefaultAddressProvider::DefaultAddressProvider(config::ClientNetworkConfig &networkConfig,
                                                               bool noOtherAddressProviderExist) : network_config_(
                        networkConfig), no_other_address_provider_exist_(noOtherAddressProviderExist) {
                }

                std::vector<Address> DefaultAddressProvider::loadAddresses() {
                    std::vector<Address> addresses = network_config_.getAddresses();
                    if (addresses.empty() && no_other_address_provider_exist_) {
                        addresses.emplace_back("127.0.0.1", 5701);
                    }

                    // TODO Implement AddressHelper to add alternative ports for the same address

                    return addresses;
                }

                const boost::shared_ptr<ClientClusterServiceImpl::member_list_snapshot> ClientClusterServiceImpl::EMPTY_SNAPSHOT(
                        new ClientClusterServiceImpl::member_list_snapshot{-1});

                constexpr boost::chrono::steady_clock::duration ClientClusterServiceImpl::INITIAL_MEMBERS_TIMEOUT;

                ClientClusterServiceImpl::ClientClusterServiceImpl(hazelcast::client::spi::ClientContext &client)
                        : client_(client), member_list_snapshot_(EMPTY_SNAPSHOT), labels_(client.getClientConfig().getLabels()),
                        initial_list_fetched_latch_(1) {
                }

                boost::uuids::uuid ClientClusterServiceImpl::addMembershipListenerWithoutInit(
                        const std::shared_ptr<MembershipListener> &listener) {
                    auto id = client_.random_uuid();
                    listeners_.put(id, listener);
                    listener->setRegistrationId(id);
                    return id;
                }

                boost::optional<Member> ClientClusterServiceImpl::getMember(boost::uuids::uuid uuid) const {
                    assert(!uuid.is_nil());
                    auto members_view_ptr = member_list_snapshot_.load();
                    const auto it = members_view_ptr->members.find(uuid);
                    if (it == members_view_ptr->members.end()) {
                        return boost::none;
                    }
                    return {it->second};
                }

                std::vector<Member> ClientClusterServiceImpl::getMemberList() const {
                    auto members_view_ptr = member_list_snapshot_.load();
                    std::vector<Member> result;
                    result.reserve(members_view_ptr->members.size());
                    for (const auto &e : members_view_ptr->members) {
                        result.emplace_back(e.second);
                    }
                    return result;
                }

                void ClientClusterServiceImpl::start() {
                    for (auto &listener : client_.getClientConfig().getManagedMembershipListeners()) {
                        addMembershipListener(listener);
                    }
                }

                void ClientClusterServiceImpl::fireInitialMembershipEvent(const InitialMembershipEvent &event) {
                    for (const std::shared_ptr<MembershipListener> &listener : listeners_.values()) {
                        if (listener->shouldRequestInitialMembers()) {
                            ((InitialMembershipListener *) listener.get())->init(event);
                        }
                    }
                }

                void ClientClusterServiceImpl::shutdown() {
                    initial_list_fetched_latch_.try_count_down();
                }

                boost::uuids::uuid
                ClientClusterServiceImpl::addMembershipListener(const std::shared_ptr<MembershipListener> &listener) {
                    util::Preconditions::checkNotNull(listener, "listener can't be null");

                    std::lock_guard<std::mutex> guard(cluster_view_lock_);
                    auto id = addMembershipListenerWithoutInit(listener);
                    if (listener->shouldRequestInitialMembers()) {
                        auto &cluster = client_.getCluster();
                        auto members_ptr = member_list_snapshot_.load();
                        if (!members_ptr->members.empty()) {
                            std::unordered_set<Member> members;
                            for (const auto &e : members_ptr->members) {
                                members.insert(e.second);
                            }
                            std::static_pointer_cast<InitialMembershipListener>(listener)->init(InitialMembershipEvent(cluster, members));
                        }
                    }

                    return id;
                }

                bool ClientClusterServiceImpl::removeMembershipListener(boost::uuids::uuid registrationId) {
                    return listeners_.remove(registrationId).get();
                }

                std::vector<Member>
                ClientClusterServiceImpl::getMembers(const cluster::memberselector::MemberSelector &selector) const {
                    std::vector<Member> result;
                    for (auto &&member : getMemberList()) {
                        if (selector.select(member)) {
                            result.emplace_back(std::move(member));
                        }
                    }

                    return result;
                }

                Client ClientClusterServiceImpl::getLocalClient() const {
                    connection::ClientConnectionManagerImpl &cm = client_.getConnectionManager();
                    auto connection = cm.get_random_connection();
                    auto inetSocketAddress = connection ? connection->getLocalSocketAddress() : boost::none;
                    auto uuid = cm.getClientUuid();
                    return Client(uuid, std::move(inetSocketAddress), client_.getName(), labels_);
                }

                void ClientClusterServiceImpl::clear_member_list_version() {
                    std::lock_guard<std::mutex> g(cluster_view_lock_);
                    auto &logger = client_.getLogger();
                    if (logger.isFinestEnabled()) {
                        logger.finest("Resetting the member list version ");
                    }
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
                    auto &logger = client_.getLogger();
                    if (logger.isFinestEnabled()) {
                        auto snapshot = create_snapshot(version, memberInfos);
                        logger.finest("Handling new snapshot with membership version: ", version, ", membersString "
                                      , members_string(snapshot));
                    }
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
                        result.members.insert({m.getUuid(), m});
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
                    client_.getLogger().info(members_string(*snapshot));
                    std::unordered_set<Member> members;
                    for(auto const &e : snapshot->members) {
                        members.insert(e.second);
                    }
                    for (auto &listener : listeners_.values()) {
                        if (listener->shouldRequestInitialMembers()) {
                            std::static_pointer_cast<InitialMembershipListener>(listener)->init(
                                    InitialMembershipEvent(client_.getCluster(), members));
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
                        events.emplace_back(client_.getCluster(), e.second, MembershipEvent::MembershipEventType::MEMBER_REMOVED, current_members);
                        auto connection = client_.getConnectionManager().getConnection(e.second.getUuid());
                        if (connection) {
                            connection->close("", std::make_exception_ptr(exception::TargetDisconnectedException(
                                    "ClientClusterServiceImpl::detect_membership_events", (boost::format(
                                            "The client has closed the connection to this member, after receiving a member left event from the cluster. %1%") %
                                                                                           *connection).str())));
                        }
                    }
                    for (auto const &member : new_members) {
                        events.emplace_back(client_.getCluster(), member, MembershipEvent::MembershipEventType::MEMBER_ADDED, current_members);
                    }

                    if (!events.empty()) {
                        auto snapshot = member_list_snapshot_.load();
                        if (!snapshot->members.empty()) {
                            client_.getLogger().info(members_string(*snapshot));
                        }
                    }
                    return events;
                }

                void ClientClusterServiceImpl::fire_events(std::vector<MembershipEvent> events) {
                    for (auto const &event : events) {
                        for (auto listener : listeners_.values()) {
                            if (event.getEventType() == MembershipEvent::MembershipEventType::MEMBER_ADDED) {
                                listener->memberAdded(event);
                            } else {
                                listener->memberRemoved(event);
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
                ClientInvocationServiceImpl::invokeOnConnection(const std::shared_ptr<ClientInvocation> &invocation,
                                                                const std::shared_ptr<connection::Connection> &connection) {
                    return send(invocation, connection);
                }

                bool ClientInvocationServiceImpl::invokeOnPartitionOwner(
                        const std::shared_ptr<ClientInvocation> &invocation, int partitionId) {
                    auto partition_owner = client_.getPartitionService().getPartitionOwner(partitionId);
                    if (partition_owner.is_nil()) {
                        if (invocation_logger_.isFinestEnabled()) {
                            invocation_logger_.finest("Partition owner is not assigned yet for partition ", partitionId);
                        }
                        return false;
                    }
                    return invokeOnTarget(invocation, partition_owner);
                }

                bool ClientInvocationServiceImpl::invokeOnTarget(const std::shared_ptr<ClientInvocation> &invocation,
                                                                 boost::uuids::uuid uuid) {
                    assert (!uuid.is_nil());
                    auto connection = client_.getConnectionManager().getConnection(uuid);
                    if (!connection) {
                        if (invocation_logger_.isFinestEnabled()) {
                            invocation_logger_.finest("Client is not connected to target : " , uuid);
                        }
                        return false;
                    }
                    return send(invocation, connection);
                }

                bool ClientInvocationServiceImpl::is_smart_routing() const {
                    return smart_routing_;
                }

                ClientExecutionServiceImpl::ClientExecutionServiceImpl(const std::string &name,
                                                                       const ClientProperties &properties,
                                                                       int32_t poolSize,
                                                                       spi::LifecycleService &service)
                        : lifecycle_service_(service), client_properties_(properties), user_executor_pool_size_(poolSize) {}

                void ClientExecutionServiceImpl::start() {
                    int internalPoolSize = client_properties_.getInteger(client_properties_.getInternalExecutorPoolSize());
                    if (internalPoolSize <= 0) {
                        internalPoolSize = util::IOUtil::to_value<int>(
                                ClientProperties::INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT);
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
                    shutdownThreadPool(user_executor_.get());
                    shutdownThreadPool(internal_executor_.get());
                }

                boost::asio::thread_pool::executor_type ClientExecutionServiceImpl::getUserExecutor() const {
                    return user_executor_->get_executor();
                }

                void ClientExecutionServiceImpl::shutdownThreadPool(hazelcast::util::hz_thread_pool *pool) {
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
                        logger_(clientContext.getLogger()),
                        lifecycle_service_(clientContext.getLifecycleService()),
                        client_cluster_service_(clientContext.getClientClusterService()),
                        invocation_service_(clientContext.getInvocationService()),
                        execution_service_(clientContext.getClientExecutionService().shared_from_this()),
                        call_id_sequence_(clientContext.getCallIdSequence()),
                        uuid_(uuid),
                        partition_id_(partition),
                        start_time_(std::chrono::steady_clock::now()),
                        retry_pause_(invocation_service_.getInvocationRetryPause()),
                        object_name_(name),
                        connection_(conn),
                        invoke_count_(0), urgent_(false), smart_routing_(invocation_service_.is_smart_routing()) {
                    message->setPartitionId(partition_id_);
                    client_message_ = boost::make_shared<std::shared_ptr<protocol::ClientMessage>>(message);
                    setSendConnection(nullptr);
                }

                ClientInvocation::~ClientInvocation() = default;

                boost::future<protocol::ClientMessage> ClientInvocation::invoke() {
                    assert (client_message_.load());
                    client_message_.load()->get()->setCorrelationId(call_id_sequence_->next());
                    invokeOnSelection();
                    return invocation_promise_.get_future().then(boost::launch::sync,
                                                               [=](boost::future<protocol::ClientMessage> f) {
                                                                   call_id_sequence_->complete();
                                                                   return f.get();
                                                               });
                }

                boost::future<protocol::ClientMessage> ClientInvocation::invokeUrgent() {
                    assert(client_message_.load());
                    urgent_ = true;
                    client_message_.load()->get()->setCorrelationId(call_id_sequence_->forceNext());
                    invokeOnSelection();
                    return invocation_promise_.get_future().then(boost::launch::sync,
                                                               [=](boost::future<protocol::ClientMessage> f) {
                                                                   call_id_sequence_->complete();
                                                                   return f.get();
                                                               });
                }

                void ClientInvocation::invokeOnSelection() {
                    try {
                        invoke_count_++;
                        if (!urgent_) {
                            invocation_service_.check_invocation_allowed();
                        }

                        if (isBindToSingleConnection()) {
                            auto invoked = invocation_service_.invokeOnConnection(shared_from_this(), connection_);
                            if (!invoked) {
                                notifyException(std::make_exception_ptr(exception::IOException("", (boost::format(
                                        "Could not invoke on connection %1%") % *connection_).str())));
                            }
                            return;
                        }

                        bool invoked = false;
                        if (smart_routing_) {
                            if (partition_id_ != -1) {
                                invoked = invocation_service_.invokeOnPartitionOwner(shared_from_this(), partition_id_);
                            } else if (!uuid_.is_nil()) {
                                invoked = invocation_service_.invokeOnTarget(shared_from_this(), uuid_);
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
                            notifyException(std::make_exception_ptr(exception::IOException("No connection found to invoke")));
                        }
                    } catch (exception::IException &) {
                        notifyException(std::current_exception());
                    } catch (std::exception &) {
                        assert(false);
                    }
                }

                bool ClientInvocation::isBindToSingleConnection() const {
                    return connection_ != nullptr;
                }

                void ClientInvocation::run() {
                    retry();
                }

                void ClientInvocation::retry() {
                    // retry modifies the client message and should not reuse the client message.
                    // It could be the case that it is in write queue of the connection.
                    client_message_ = boost::make_shared<std::shared_ptr<protocol::ClientMessage>>(copyMessage());

                    try {
                        invokeOnSelection();
                    } catch (exception::IException &e) {
                        setException(e, boost::current_exception());
                    } catch (std::exception &) {
                        assert(false);
                    }
                }

                void ClientInvocation::setException(const exception::IException &e, boost::exception_ptr exceptionPtr) {
                    try {
                        auto send_conn = *send_connection_.load();
                        if (send_conn) {
                            auto call_id = client_message_.load()->get()->getCorrelationId();
                            boost::asio::post(send_conn->getSocket().get_executor(), [=] () {
                                send_conn->deregisterInvocation(call_id);
                            });
                        }
                        invocation_promise_.set_exception(std::move(exceptionPtr));
                    } catch (boost::promise_already_satisfied &se) {
                        if (!event_handler_) {
                            logger_.warning("Failed to set the exception for invocation. ", se.what(), ", ", *this,
                                           " Exception to be set: ", e);
                        }
                    }
                }

                void ClientInvocation::notifyException(std::exception_ptr exception) {
                    try {
                        std::rethrow_exception(exception);
                    } catch (exception::IException &iex) {
                        log_exception(iex);

                        if (!lifecycle_service_.isRunning()) {
                            try {
                                std::throw_with_nested(boost::enable_current_exception(
                                        exception::HazelcastClientNotActiveException(iex.getSource(),
                                                                                     "Client is shutting down")));
                            } catch (exception::IException &e) {
                                setException(e, boost::current_exception());
                            }
                            return;
                        }

                        if (!should_retry(iex)) {
                            setException(iex, boost::current_exception());
                            return;
                        }

                        auto timePassed = std::chrono::steady_clock::now() - start_time_;
                        if (timePassed > invocation_service_.getInvocationTimeout()) {
                            if (logger_.isFinestEnabled()) {
                                std::ostringstream out;
                                out << "Exception will not be retried because invocation timed out. " << iex.what();
                                logger_.finest(out.str());
                            }

                            auto now = std::chrono::steady_clock::now();

                            auto timeoutException = (exception::ExceptionBuilder<exception::OperationTimeoutException>(
                                    "ClientInvocation::newOperationTimeoutException") << *this
                                            << " timed out because exception occurred after client invocation timeout "
                                            << std::chrono::duration_cast<std::chrono::milliseconds>(invocation_service_.getInvocationTimeout()).count()
                                            << "msecs. Last exception:" << iex
                                            << " Current time :" << util::StringUtil::timeToString(now) << ". "
                                            << "Start time: " << util::StringUtil::timeToString(start_time_)
                                            << ". Total elapsed time: " <<
                                            std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count()
                                            << " ms. ").build();
                            try {
                                BOOST_THROW_EXCEPTION(timeoutException);
                            } catch (...) {
                                setException(timeoutException, boost::current_exception());
                            }

                            return;
                        }

                        try {
                            execute();
                        } catch (exception::IException &e) {
                            setException(e, boost::current_exception());
                        }
                    } catch (...) {
                        assert(false);
                    }
                }

                bool ClientInvocation::should_retry(exception::IException &exception) {
                    auto errorCode = exception.getErrorCode();
                    if (isBindToSingleConnection() && (errorCode == protocol::IO || errorCode == protocol::TARGET_DISCONNECTED)) {
                        return false;
                    }

                    if (!uuid_.is_nil() && errorCode == protocol::TARGET_NOT_MEMBER) {
                        //when invocation send to a specific member
                        //if target is no longer a member, we should not retry
                        //note that this exception could come from the server
                        return false;
                    }

                    if (errorCode == protocol::IO || errorCode == protocol::HAZELCAST_INSTANCE_NOT_ACTIVE || exception.isRetryable()) {
                        return true;
                    }
                    if (errorCode == protocol::TARGET_DISCONNECTED) {
                        return client_message_.load()->get()->isRetryable() || invocation_service_.isRedoOperation();
                    }
                    return false;
                }

                std::ostream &operator<<(std::ostream &os, const ClientInvocation &invocation) {
                    std::ostringstream target;
                    if (invocation.isBindToSingleConnection()) {
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
                    auto sendConnection = invocation.getSendConnection();
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

                std::shared_ptr<connection::Connection> ClientInvocation::getSendConnection() const {
                    return *send_connection_.load();
                }

                std::shared_ptr<connection::Connection> ClientInvocation::getSendConnectionOrWait() const {
                    while (!(*send_connection_.load()) && lifecycle_service_.isRunning()) {
                        std::this_thread::yield();
                    }
                    if (!lifecycle_service_.isRunning()) {
                        BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("Client is being shut down!"));
                    }
                    return *send_connection_.load();
                }

                void
                ClientInvocation::setSendConnection(const std::shared_ptr<connection::Connection> &conn) {
                    send_connection_.store(boost::make_shared<std::shared_ptr<connection::Connection>>(conn));
                }

                void ClientInvocation::notify(const std::shared_ptr<protocol::ClientMessage> &msg) {
                    if (!msg) {
                        BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("response can't be null"));
                    }
                    try {
                        // TODO: move msg content here?
                        invocation_promise_.set_value(*msg);
                    } catch (std::exception &e) {
                        logger_.warning("Failed to set the response for invocation. Dropping the response. ", e.what(),
                                       ", ",*this, " Response: ", *msg);
                    }
                }

                std::shared_ptr<protocol::ClientMessage> ClientInvocation::getClientMessage() const {
                    return *client_message_.load();
                }

                const std::shared_ptr<EventHandler<protocol::ClientMessage> > &
                ClientInvocation::getEventHandler() const {
                    return event_handler_;
                }

                void ClientInvocation::setEventHandler(
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
                    int64_t callId = call_id_sequence_->forceNext();
                    client_message_.load()->get()->setCorrelationId(callId);

                    //we release the old slot
                    call_id_sequence_->complete();

                    if (invoke_count_ < MAX_FAST_INVOCATION_COUNT) {
                        // fast retry for the first few invocations
                        execution_service_->execute(command);
                    } else {
                        // progressive retry delay
                        int64_t delayMillis = util::min<int64_t>(static_cast<int64_t>(1) << (invoke_count_ - MAX_FAST_INVOCATION_COUNT),
                                                                 std::chrono::duration_cast<std::chrono::milliseconds>(retry_pause_).count());
                        execution_service_->schedule(command, std::chrono::milliseconds(delayMillis));
                    }
                }

                const std::string ClientInvocation::getName() const {
                    return "ClientInvocation";
                }

                std::shared_ptr<protocol::ClientMessage> ClientInvocation::copyMessage() {
                    return std::make_shared<protocol::ClientMessage>(**client_message_.load());
                }

                boost::promise<protocol::ClientMessage> &ClientInvocation::getPromise() {
                    return invocation_promise_;
                }

                void ClientInvocation::log_exception(exception::IException &e) {
                    if (logger_.isFinestEnabled()) {
                        logger_.finest("Invocation got an exception ", *this, ", invoke count : ", invoke_count_.load(),
                                      ", exception : ", e);
                    }
                }

                ClientContext &impl::ClientTransactionManagerServiceImpl::getClient() const {
                    return client_;
                }

                ClientTransactionManagerServiceImpl::ClientTransactionManagerServiceImpl(ClientContext &client)
                        : client_(client) {}

                std::shared_ptr<connection::Connection> ClientTransactionManagerServiceImpl::connect() {
                    auto &invocationService = client_.getInvocationService();
                    auto startTime = std::chrono::steady_clock::now();
                    auto invocationTimeout = invocationService.getInvocationTimeout();
                    ClientConfig &clientConfig = client_.getClientConfig();
                    bool smartRouting = clientConfig.getNetworkConfig().isSmartRouting();

                    while (client_.getLifecycleService().isRunning()) {
                        try {
                            auto connection = client_.getConnectionManager().get_random_connection();
                            if (!connection) {
                                throw_exception(smartRouting);
                            }
                            return connection;
                        } catch (exception::HazelcastClientOfflineException &) {
                            throw;
                        } catch (exception::IException &) {
                            if (std::chrono::steady_clock::now() - startTime > invocationTimeout) {
                                std::rethrow_exception(
                                        newOperationTimeoutException(std::current_exception(), invocationTimeout,
                                                                     startTime));
                            }
                        }
                        std::this_thread::sleep_for(invocationService.getInvocationRetryPause());
                    }
                    BOOST_THROW_EXCEPTION(
                            exception::HazelcastClientNotActiveException("ClientTransactionManagerServiceImpl::connect",
                                                                         "Client is shutdown"));
                }

                std::exception_ptr
                ClientTransactionManagerServiceImpl::newOperationTimeoutException(std::exception_ptr cause,
                                                                                  std::chrono::steady_clock::duration invocationTimeout,
                                                                                  std::chrono::steady_clock::time_point startTime) {
                    std::ostringstream sb;
                    auto now = std::chrono::steady_clock::now();
                    sb
                            << "Creating transaction context timed out because exception occurred after client invocation timeout "
                            << std::chrono::duration_cast<std::chrono::milliseconds>(invocationTimeout).count() << " ms. " << "Current time: "
                            << util::StringUtil::timeToString(std::chrono::steady_clock::now()) << ". " << "Start time: "
                            << util::StringUtil::timeToString(startTime) << ". Total elapsed time: "
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
                    auto &client_config = client_.getClientConfig();
                    auto &connection_strategy_Config = client_config.getConnectionStrategyConfig();
                    auto reconnect_mode = connection_strategy_Config.getReconnectMode();
                    if (reconnect_mode == config::ClientConnectionStrategyConfig::ReconnectMode::ASYNC) {
                        BOOST_THROW_EXCEPTION(exception::HazelcastClientOfflineException(
                                "ClientTransactionManagerServiceImpl::throw_exception", ""));
                    }
                    if (smart_routing) {
                        auto members = client_.getCluster().getMembers();
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
                                                       util::ILogger &logger) : aws_member_port_(
                        util::IOUtil::to_string<int>(awsMemberPort)), logger_(logger), aws_client_(awsConfig, logger) {
                }

                std::vector<Address> AwsAddressProvider::loadAddresses() {
                    updateLookupTable();
                    std::unordered_map<std::string, std::string> lookupTable = getLookupTable();
                    std::vector<Address> addresses;

                    typedef std::unordered_map<std::string, std::string> LookupTable;
                    for (const LookupTable::value_type &privateAddress : lookupTable) {
                        std::vector<Address> possibleAddresses = util::AddressHelper::getSocketAddresses(
                                privateAddress.first + ":" + aws_member_port_, logger_);
                        addresses.insert(addresses.begin(), possibleAddresses.begin(),
                                         possibleAddresses.end());
                    }
                    return addresses;
                }

                void AwsAddressProvider::updateLookupTable() {
                    try {
                        private_to_public_ = aws_client_.getAddresses();
                    } catch (exception::IException &e) {
                        logger_.warning("Aws addresses failed to load: ", e.getMessage());
                    }
                }

                std::unordered_map<std::string, std::string> AwsAddressProvider::getLookupTable() {
                    return private_to_public_;
                }

                AwsAddressProvider::~AwsAddressProvider() = default;

                Address DefaultAddressTranslator::translate(const Address &address) {
                    return address;
                }

                void DefaultAddressTranslator::refresh() {
                }


                ClientPartitionServiceImpl::ClientPartitionServiceImpl(ClientContext &client)
                        : client_(client), logger_(client.getLogger()), partition_count_(0),
                        partition_table_(boost::shared_ptr<partition_table>(new partition_table{nullptr, -1})) {
                }

                void ClientPartitionServiceImpl::handle_event(const std::shared_ptr<connection::Connection>& connection, int32_t version,
                                                              const std::vector<std::pair<boost::uuids::uuid, std::vector<int>>> &partitions) {
                    if (logger_.isFinestEnabled()) {
                        logger_.finest("Handling new partition table with  partitionStateVersion: " , version);
                    }
                    while (true) {
                        auto current = partition_table_.load();
                        if (!should_be_applied(connection, version, partitions, *current)) {
                            return;
                        }
                        if (partition_table_.compare_exchange_strong(current, boost::shared_ptr<partition_table>(
                                new partition_table{connection, version, convert_to_map(partitions)}))) {
                            if (logger_.isFinestEnabled()) {
                                logger_.finest("Applied partition table with partitionStateVersion : ", version);
                            }
                            return;
                        }

                    }
                }

                boost::uuids::uuid ClientPartitionServiceImpl::getPartitionOwner(int32_t partitionId) {
                    auto table_ptr = partition_table_.load();
                    auto it = table_ptr->partitions.find(partitionId);
                    if (it != table_ptr->partitions.end()) {
                        return it->second;
                    }
                    return boost::uuids::nil_uuid();
                }

                int32_t ClientPartitionServiceImpl::getPartitionId(const serialization::pimpl::Data &key) {
                    int32_t pc = getPartitionCount();
                    if (pc <= 0) {
                        return 0;
                    }
                    int hash = key.getPartitionHash();
                    return util::HashUtil::hashToIndex(hash, pc);
                }

                int32_t ClientPartitionServiceImpl::getPartitionCount() {
                    return partition_count_.load();
                }

                std::shared_ptr<client::impl::Partition> ClientPartitionServiceImpl::getPartition(int partitionId) {
                    return std::shared_ptr<client::impl::Partition>(new PartitionImpl(partitionId, client_, *this));
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
                    auto &logger = client_.getLogger();
                    if (partitions.empty()) {
                        if (logger.isFinestEnabled()) {
                            log_failure(connection, version, current, "response is empty");
                        }
                        return false;
                    }
                    if (!current.connection || *connection != *current.connection) {
                        if (logger.isFinestEnabled()) {
                            if (current.connection) {
                                logger.finest("Event coming from a new connection. Old connection: ", *current.connection
                                        , ", new connection ", *connection);
                            } else {
                                logger.finest(
                                        "Event coming from a new connection. Old connection: nullptr, new connection ",
                                        *connection);
                            }
                        }
                        return true;
                    }
                    if (version <= current.version) {
                        if (logger.isFinestEnabled()) {
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
                    if (logger_.isFinestEnabled()) {
                        if (current.connection) {
                            logger_.finest(" We will not apply the response, since " + cause + " . Response is from ", *connection
                                    , ". Current connection ", *current.connection
                                    , " response state version:", version
                                    , ". Current state version: ", current.version);
                        } else {
                            logger_.finest(" We will not apply the response, since " + cause + " . Response is from ", *connection
                                    , ". Current connection : nnullptr, response state version:", version
                                    , ". Current state version: ", current.version);
                        }
                    }
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

                int ClientPartitionServiceImpl::PartitionImpl::getPartitionId() const {
                    return partition_id_;
                }

                boost::optional<Member> ClientPartitionServiceImpl::PartitionImpl::getOwner() const {
                    auto owner = partition_service_.getPartitionOwner(partition_id_);
                    if (!owner.is_nil()) {
                        return client_.getClientClusterService().getMember(owner);
                    }
                    return boost::none;
                }

                ClientPartitionServiceImpl::PartitionImpl::PartitionImpl(int partitionId, ClientContext &client,
                                                                         ClientPartitionServiceImpl &partitionService)
                        : partition_id_(partitionId), client_(client), partition_service_(partitionService) {
                }

                namespace sequence {
                    CallIdSequenceWithoutBackpressure::CallIdSequenceWithoutBackpressure() : head_(0) {}

                    CallIdSequenceWithoutBackpressure::~CallIdSequenceWithoutBackpressure() = default;

                    int32_t CallIdSequenceWithoutBackpressure::getMaxConcurrentInvocations() const {
                        return INT32_MAX;
                    }

                    int64_t CallIdSequenceWithoutBackpressure::next() {
                        return forceNext();
                    }

                    int64_t CallIdSequenceWithoutBackpressure::forceNext() {
                        return ++head_;
                    }

                    void CallIdSequenceWithoutBackpressure::complete() {
                        // no-op
                    }

                    int64_t CallIdSequenceWithoutBackpressure::getLastCallId() {
                        return head_;
                    }

                    // TODO: see if we can utilize std::hardware_destructive_interference_size
                    AbstractCallIdSequence::AbstractCallIdSequence(int32_t maxConcurrentInvocations) {
                        std::ostringstream out;
                        out << "maxConcurrentInvocations should be a positive number. maxConcurrentInvocations="
                            << maxConcurrentInvocations;
                        this->max_concurrent_invocations_ = util::Preconditions::checkPositive(maxConcurrentInvocations,
                                                                                            out.str());

                        for (size_t i = 0; i < longs_.size(); ++i) {
                            longs_[i] = 0;
                        }
                    }

                    AbstractCallIdSequence::~AbstractCallIdSequence() = default;

                    int32_t AbstractCallIdSequence::getMaxConcurrentInvocations() const {
                        return max_concurrent_invocations_;
                    }

                    int64_t AbstractCallIdSequence::next() {
                        if (!hasSpace()) {
                            handleNoSpaceLeft();
                        }
                        return forceNext();
                    }

                    int64_t AbstractCallIdSequence::forceNext() {
                        return ++longs_[INDEX_HEAD];
                    }

                    void AbstractCallIdSequence::complete() {
                        ++longs_[INDEX_TAIL];
                        assert(longs_[INDEX_TAIL] <= longs_[INDEX_HEAD]);
                    }

                    int64_t AbstractCallIdSequence::getLastCallId() {
                        return longs_[INDEX_HEAD];
                    }

                    bool AbstractCallIdSequence::hasSpace() {
                        return longs_[INDEX_HEAD] - longs_[INDEX_TAIL] < max_concurrent_invocations_;
                    }

                    int64_t AbstractCallIdSequence::getTail() {
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
                        util::Preconditions::checkPositive(backoffTimeoutMs, out.str());

                        backoff_timeout_nanos_ = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                std::chrono::milliseconds(backoffTimeoutMs)).count();
                    }

                    void CallIdSequenceWithBackpressure::handleNoSpaceLeft() {
                        auto start = std::chrono::steady_clock::now();
                        for (int64_t idleCount = 0;; idleCount++) {
                            int64_t elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                    std::chrono::steady_clock::now() - start).count();
                            if (elapsedNanos > backoff_timeout_nanos_) {
                                throw (exception::ExceptionBuilder<exception::HazelcastOverloadException>(
                                        "CallIdSequenceWithBackpressure::handleNoSpaceLeft")
                                        << "Timed out trying to acquire another call ID."
                                        << " maxConcurrentInvocations = " << getMaxConcurrentInvocations()
                                        << ", backoffTimeout = "
                                        << std::chrono::microseconds(backoff_timeout_nanos_ / 1000).count()
                                        << " msecs, elapsed:"
                                        << std::chrono::microseconds(elapsedNanos / 1000).count() << " msecs").build();
                            }
                            IDLER->idle(idleCount);
                            if (hasSpace()) {
                                return;
                            }

                        }
                    }

                    FailFastCallIdSequence::FailFastCallIdSequence(int32_t maxConcurrentInvocations)
                            : AbstractCallIdSequence(maxConcurrentInvocations) {}

                    void FailFastCallIdSequence::handleNoSpaceLeft() {
                        throw (exception::ExceptionBuilder<exception::HazelcastOverloadException>(
                                "FailFastCallIdSequence::handleNoSpaceLeft")
                                << "Maximum invocation count is reached. maxConcurrentInvocations = "
                                << getMaxConcurrentInvocations()).build();

                    }

                    std::unique_ptr<CallIdSequence> CallIdFactory::newCallIdSequence(bool isBackPressureEnabled,
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
                            : client_context_(clientContext),
                              serialization_service_(clientContext.getSerializationService()),
                              logger_(clientContext.getLogger()),
                              client_connection_manager_(clientContext.getConnectionManager()),
                              number_of_event_threads_(eventThreadCount),
                              smart_(clientContext.getClientConfig().getNetworkConfig().isSmartRouting()) {
                        auto &invocationService = clientContext.getInvocationService();
                        invocation_timeout_ = invocationService.getInvocationTimeout();
                        invocation_retry_pause_ = invocationService.getInvocationRetryPause();
                    }

                    bool listener_service_impl::registersLocalOnly() const {
                        return smart_;
                    }

                    boost::future<boost::uuids::uuid>
                    listener_service_impl::registerListener(
                            std::shared_ptr<ListenerMessageCodec> listenerMessageCodec,
                            std::shared_ptr<client::impl::BaseEventHandler> handler) {
                        auto task = boost::packaged_task<boost::uuids::uuid()>(
                                [=]() {
                                    return registerListenerInternal(listenerMessageCodec, handler);
                                });
                        auto f = task.get_future();
                        boost::asio::post(registration_executor_->get_executor(), std::move(task));
                        return f;
                    }

                    boost::future<bool> listener_service_impl::deregisterListener(boost::uuids::uuid registrationId) {
                        util::Preconditions::checkNotNill(registrationId, "Nil userRegistrationId is not allowed!");

                        boost::packaged_task<bool()> task([=]() {
                            return deregisterListenerInternal(registrationId);
                        });
                        auto f = task.get_future();
                        boost::asio::post(registration_executor_->get_executor(), std::move(task));
                        return f;
                    }

                    void listener_service_impl::connectionAdded(
                            const std::shared_ptr<connection::Connection> connection) {
                        boost::asio::post(registration_executor_->get_executor(), [=]() { connectionAddedInternal(connection); });
                    }

                    void listener_service_impl::connectionRemoved(
                            const std::shared_ptr<connection::Connection> connection) {
                        boost::asio::post(registration_executor_->get_executor(), [=]() { connectionRemovedInternal(connection); });
                    }

                    void
                    listener_service_impl::removeEventHandler(int64_t call_id,
                                                              const std::shared_ptr<connection::Connection> &connection) {
                        boost::asio::post(connection->getSocket().get_executor(),
                                          std::packaged_task<void()>([=]() {
                                              connection->deregisterInvocation(call_id);
                                          }));
                    }

                    void listener_service_impl::handleClientMessage(
                            const std::shared_ptr<ClientInvocation> invocation,
                            const std::shared_ptr<protocol::ClientMessage> response) {
                        try {
                            auto partitionId = response->getPartitionId();
                            if (partitionId == -1) {
                                // execute on random thread on the thread pool
                                boost::asio::post(event_executor_->get_executor(), [=]() { processEventMessage(invocation, response); });
                                return;
                            }

                            // process on certain thread which is same for the partition id
                            boost::asio::post(event_strands_[partitionId % event_strands_.size()],
                                              [=]() { processEventMessage(invocation, response); });

                        } catch (const std::exception &e) {
                            if (client_context_.getLifecycleService().isRunning()) {
                                logger_.warning("Delivery of event message to event handler failed. ", e.what(),
                                               ", *response, "", ", *invocation);
                            }
                        }
                    }

                    void listener_service_impl::shutdown() {
                        event_strands_.clear();
                        ClientExecutionServiceImpl::shutdownThreadPool(event_executor_.get());
                        ClientExecutionServiceImpl::shutdownThreadPool(registration_executor_.get());
                    }

                    void listener_service_impl::start() {
                        event_executor_.reset(new hazelcast::util::hz_thread_pool(number_of_event_threads_));
                        registration_executor_.reset(new hazelcast::util::hz_thread_pool(1));

                        for (int i = 0; i < number_of_event_threads_; ++i) {
                            event_strands_.emplace_back(event_executor_->get_executor());
                        }

                        client_connection_manager_.addConnectionListener(shared_from_this());
                    }

                    boost::uuids::uuid listener_service_impl::registerListenerInternal(
                            std::shared_ptr<ListenerMessageCodec> listenerMessageCodec,
                            std::shared_ptr<client::impl::BaseEventHandler> handler) {
                        auto user_registration_id = client_context_.random_uuid();

                        std::shared_ptr<listener_registration> registration(new listener_registration{listenerMessageCodec, handler});
                        registrations_.put(user_registration_id, registration);
                        for (auto const &connection : client_connection_manager_.getActiveConnections()) {
                            try {
                                invoke(registration, connection);
                            } catch (exception::IException &e) {
                                if (connection->isAlive()) {
                                    deregisterListenerInternal(user_registration_id);
                                    BOOST_THROW_EXCEPTION((exception::ExceptionBuilder<exception::HazelcastException>(
                                            "ClientListenerService::RegisterListenerTask::call")
                                            << "Listener can not be added " << e).build());
                                }
                            }
                        }
                        return user_registration_id;
                    }

                    bool
                    listener_service_impl::deregisterListenerInternal(boost::uuids::uuid userRegistrationId) {
                        auto listenerRegistration = registrations_.get(userRegistrationId);
                        if (!listenerRegistration) {
                            return false;
                        }
                        bool successful = true;

                        auto listener_registrations = listenerRegistration->registrations.entrySet();
                        for (auto it = listener_registrations.begin();it != listener_registrations.end();) {
                            const auto &registration = it->second;
                            const auto& subscriber = it->first;
                            try {
                                const auto &listenerMessageCodec = listenerRegistration->codec;
                                auto serverRegistrationId = registration->server_registration_id;
                                auto request = listenerMessageCodec->encodeRemoveRequest(serverRegistrationId);
                                auto invocation = ClientInvocation::create(client_context_,request, "",
                                                                           subscriber);
                                invocation->invoke().get();

                                removeEventHandler(registration->call_id, subscriber);

                                it = listener_registrations.erase(it);
                            } catch (exception::IException &e) {
                                ++it;
                                if (subscriber->isAlive()) {
                                    successful = false;
                                    std::ostringstream endpoint;
                                    if (subscriber->getRemoteAddress()) {
                                        endpoint << *subscriber->getRemoteAddress();
                                    } else {
                                        endpoint << "null";
                                    }
                                    logger_.warning("ClientListenerService::deregisterListenerInternal",
                                                   "Deregistration of listener with ID ", userRegistrationId,
                                                   " has failed to address ", subscriber->getRemoteAddress(), e);
                                }
                            }
                        }
                        if (successful) {
                            registrations_.remove(userRegistrationId);
                        }
                        return successful;
                    }

                    void listener_service_impl::connectionAddedInternal(
                            const std::shared_ptr<connection::Connection> &connection) {
                        for (const auto &listener_registration : registrations_.values()) {
                            invokeFromInternalThread(listener_registration, connection);
                        }
                    }

                    void listener_service_impl::connectionRemovedInternal(
                            const std::shared_ptr<connection::Connection> &connection) {
                        for (auto &registry : registrations_.values()) {
                            registry->registrations.remove(connection);
                        }
                    }

                    void
                    listener_service_impl::invokeFromInternalThread(
                            const std::shared_ptr<listener_registration> &listener_registration,
                            const std::shared_ptr<connection::Connection> &connection) {
                        try {
                            invoke(listener_registration, connection);
                        } catch (exception::IException &e) {
                            logger_.warning("Listener with pointer", listener_registration.get(),
                                           " can not be added to a new connection: ", *connection, ", reason: ", e);
                        }
                    }

                    void
                    listener_service_impl::invoke(const std::shared_ptr<listener_registration> &listener_registration,
                                                  const std::shared_ptr<connection::Connection> &connection) {
                        if (listener_registration->registrations.containsKey(connection)) {
                            return;
                        }

                        const auto &codec = listener_registration->codec;
                        auto request = codec->encodeAddRequest(registersLocalOnly());
                        const auto &handler = listener_registration->handler;
                        handler->beforeListenerRegister();

                        auto invocation = ClientInvocation::create(client_context_,
                                                                   std::make_shared<protocol::ClientMessage>(std::move(request)), "",
                                                                   connection);
                        invocation->setEventHandler(handler);
                        auto clientMessage = invocation->invokeUrgent().get();

                        auto serverRegistrationId = codec->decodeAddResponse(clientMessage);
                        handler->onListenerRegister();
                        int64_t correlationId = invocation->getClientMessage()->getCorrelationId();

                        (*listener_registration).registrations.put(connection, std::shared_ptr<connection_registration>(
                                new connection_registration{serverRegistrationId, correlationId}));
                    }

                    void listener_service_impl::processEventMessage(
                            const std::shared_ptr<ClientInvocation> invocation,
                            const std::shared_ptr<protocol::ClientMessage> response) {
                        auto eventHandler = invocation->getEventHandler();
                        if (!eventHandler) {
                            if (client_context_.getLifecycleService().isRunning()) {
                                logger_.warning("No eventHandler for invocation. Ignoring this invocation response. ",
                                               *invocation);
                            }

                            return;
                        }

                        try {
                            eventHandler->handle(*response);
                        } catch (std::exception &e) {
                            if (client_context_.getLifecycleService().isRunning()) {
                                logger_.warning("Delivery of event message to event handler failed. ", e.what(),
                                               ", *response, "", ", *invocation);
                            }
                        }
                    }

                    listener_service_impl::~listener_service_impl() = default;

                    void cluster_view_listener::start() {
                        client_context_.getConnectionManager().addConnectionListener(shared_from_this());
                    }

                    void cluster_view_listener::connectionAdded(const std::shared_ptr<connection::Connection> connection) {
                        try_register(connection);
                    }

                    void cluster_view_listener::connectionRemoved(const std::shared_ptr<connection::Connection> connection) {
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
                        invocation->setEventHandler(handler);
                        handler->beforeListenerRegister();

                        invocation->invokeUrgent().then([=] (boost::future<protocol::ClientMessage> f) {
                            if (f.has_value()) {
                                handler->onListenerRegister();
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
                        auto new_connection = client_context_.getConnectionManager().get_random_connection();
                        if (new_connection) {
                            try_register(new_connection);
                        }
                    }

                    cluster_view_listener::~cluster_view_listener() = default;

                    void
                    cluster_view_listener::event_handler::handle_membersview(int32_t version,
                                                                             const std::vector<Member> &memberInfos) {
                        view_listener_.client_context_.getClientClusterService().handle_event(version, memberInfos);
                    }

                    void
                    cluster_view_listener::event_handler::handle_partitionsview(int32_t version,
                                                                                const std::vector<std::pair<boost::uuids::uuid, std::vector<int>>> &partitions) {
                        view_listener_.client_context_.getPartitionService().handle_event(connection_, version, partitions);
                    }

                    void cluster_view_listener::event_handler::beforeListenerRegister() {
                        view_listener_.client_context_.getClientClusterService().clear_member_list_version();
                        auto &logger = view_listener_.client_context_.getLogger();
                        if (logger.isFinestEnabled()) {
                            logger.finest("Register attempt of ClusterViewListenerHandler to ", *connection_);
                        }
                    }

                    void cluster_view_listener::event_handler::onListenerRegister() {
                        auto &logger = view_listener_.client_context_.getLogger();
                        if (logger.isFinestEnabled()) {
                            logger.finest("Registered ClusterViewListenerHandler to ", *connection_);
                        }
                    }

                    cluster_view_listener::event_handler::event_handler(
                            const std::shared_ptr<connection::Connection> &connection,
                            cluster_view_listener &viewListener) : connection_(connection),
                                                                   view_listener_(viewListener) {}
                }

            }
        }
    }
}

namespace std {
    bool less<hazelcast::client::spi::DefaultObjectNamespace>::operator()(
            const hazelcast::client::spi::DefaultObjectNamespace &lhs,
            const hazelcast::client::spi::DefaultObjectNamespace &rhs) const {
        int result = lhs.getServiceName().compare(rhs.getServiceName());
        if (result < 0) {
            return true;
        }

        if (result > 0) {
            return false;
        }

        return lhs.getObjectName().compare(rhs.getObjectName()) < 0;
    }

    std::size_t
    hash<hazelcast::client::spi::DefaultObjectNamespace>::operator()(
            const hazelcast::client::spi::DefaultObjectNamespace &k) const noexcept {
        return std::hash<std::string>()(k.getServiceName() + k.getObjectName());
    }
}



