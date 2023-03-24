/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include <cstdlib>
#include <unordered_set>
#include <boost/algorithm/string/join.hpp>

#include "hazelcast/client/execution_callback.h"
#include "hazelcast/client/lifecycle_event.h"
#include "hazelcast/client/connection/AddressProvider.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/protocol/AuthenticationStatus.h"
#include "hazelcast/client/exception/protocol_exceptions.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/connection/ConnectionListener.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/protocol/UsernamePasswordCredentials.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/client_config.h"
#include "hazelcast/client/socket_interceptor.h"
#include "hazelcast/client/config/client_network_config.h"
#include "hazelcast/client/client_properties.h"
#include "hazelcast/client/connection/HeartbeatManager.h"
#include "hazelcast/client/impl/hazelcast_client_instance_impl.h"
#include "hazelcast/client/spi/impl/listener/listener_service_impl.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientInvocationServiceImpl.h"
#include "hazelcast/client/internal/socket/TcpSocket.h"
#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/config/ssl_config.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/internal/socket/SocketFactory.h"
#include "hazelcast/client/connection/wait_strategy.h"
#include "hazelcast/client/connection/tpc_channel_connector.h"

namespace hazelcast {
namespace client {

std::ostream&
operator<<(std::ostream& os, const socket& channel)
{
    return os << "Socket {" << channel.local_socket_address() << "->"
              << channel.get_remote_endpoint() << "}";
}

namespace connection {

authentication_response::authentication_response(
  byte auth_status,
  boost::optional<address> address,
  boost::uuids::uuid member_uuid,
  byte serialization_version,
  std::string server_version,
  int32_t partition_count,
  boost::uuids::uuid cluster_id,
  std::vector<int> tpc_ports)
  : status_{ auth_status }
  , server_address_{ address }
  , member_uuid_{ member_uuid }
  , serialization_version_{ serialization_version }
  , server_version_{ server_version }
  , partition_count_{ partition_count }
  , cluster_id_{ cluster_id }
  , tpc_ports_{ move(tpc_ports) }
{
}

authentication_response
authentication_response::from(protocol::ClientMessage response)
{
    switch (response.get_message_type()) {
        case AUTHENTICATION_CODEC_RESPONSE_MESSAGE_TYPE:
            return from_authentication_codec(std::move(response));
        case AUTHENTICATION_CUSTOM_CODEC_RESPONSE_MESSAGE_TYPE:
            return from_authentication_custom_codec(std::move(response));
        case EXPERIMENTAL_AUTHENTICATION_CODEC_RESPONSE_MESSAGE_TYPE:
            return from_experimental_authentication_codec(std::move(response));
        case EXPERIMENTAL_AUTHENTICATION_CUSTOM_CODEC_RESPONSE_MESSAGE_TYPE:
            return from_experimental_authentication_custom_codec(
              std::move(response));
    }

    BOOST_THROW_EXCEPTION(
      exception::illegal_state{ "Unexpected response message type" });
}

byte
authentication_response::status() const
{
    return status_;
}

boost::optional<address>
authentication_response::addr() const
{
    return server_address_;
}

boost::uuids::uuid
authentication_response::member_uuid() const
{
    return member_uuid_;
}

byte
authentication_response::serialization_version() const
{
    return serialization_version_;
}

const std::string&
authentication_response::server_hazelcast_version() const
{
    return server_version_;
}

int
authentication_response::partition_count() const
{
    return partition_count_;
}

boost::uuids::uuid
authentication_response::cluster_id() const
{
    return cluster_id_;
}

const std::vector<int>&
authentication_response::tpc_ports() const
{
    return tpc_ports_;
}

authentication_response
authentication_response::from_authentication_codec(
  protocol::ClientMessage response)
{
    return decode(std::move(response), false);
}

authentication_response
authentication_response::from_authentication_custom_codec(
  protocol::ClientMessage response)
{
    return decode(std::move(response), false);
}

authentication_response
authentication_response::from_experimental_authentication_codec(
  protocol::ClientMessage response)
{
    return decode(std::move(response), true);
}

authentication_response
authentication_response::from_experimental_authentication_custom_codec(
  protocol::ClientMessage response)
{
    return decode(std::move(response), true);
}

authentication_response
authentication_response::decode(protocol::ClientMessage response,
                                bool with_tpc_ports)
{
    auto* initial_frame =
      reinterpret_cast<protocol::ClientMessage::frame_header_type*>(
        response.rd_ptr(protocol::ClientMessage::RESPONSE_HEADER_LEN));

    auto status = response.get<byte>();
    auto member_uuid = response.get<boost::uuids::uuid>();
    auto serialization_version = response.get<byte>();
    auto partition_count = response.get<int32_t>();
    auto cluster_id = response.get<boost::uuids::uuid>();

    // skip first frame
    response.rd_ptr(
      static_cast<int32_t>(initial_frame->frame_len) -
      protocol::ClientMessage::RESPONSE_HEADER_LEN -
      2 * protocol::ClientMessage::UINT8_SIZE -
      2 * (sizeof(boost::uuids::uuid) + protocol::ClientMessage::UINT8_SIZE) -
      protocol::ClientMessage::INT32_SIZE);
    auto server_address = response.get_nullable<address>();
    auto server_version = response.get<std::string>();
    std::vector<int> tpc_ports;

    if (with_tpc_ports) {
        auto ports = response.get_nullable<std::vector<int>>();

        if (ports) {
            tpc_ports = std::move(*ports);
        }
    }

    return authentication_response{ status,
                                    server_address,
                                    member_uuid,
                                    serialization_version,
                                    std::move(server_version),
                                    partition_count,
                                    cluster_id,
                                    move(tpc_ports) };
}

constexpr size_t ClientConnectionManagerImpl::EXECUTOR_CORE_POOL_SIZE;
const endpoint_qualifier ClientConnectionManagerImpl::PUBLIC_ENDPOINT_QUALIFIER{
    CLIENT,
    "public"
};

constexpr int ClientConnectionManagerImpl::SQL_CONNECTION_RANDOM_ATTEMPTS;

ClientConnectionManagerImpl::ClientConnectionManagerImpl(
  spi::ClientContext& client,
  std::unique_ptr<AddressProvider> address_provider)
  : alive_(false)
  , logger_(client.get_logger())
  , connection_timeout_millis_((std::chrono::milliseconds::max)())
  , client_(client)
  , socket_interceptor_(client.get_client_config().get_socket_interceptor())
  , address_provider_(std::move(address_provider))
  , connection_id_gen_(0)
  , heartbeat_(client, *this)
  , async_start_(client.get_client_config()
                   .get_connection_strategy_config()
                   .is_async_start())
  , reconnect_mode_(client.get_client_config()
                      .get_connection_strategy_config()
                      .get_reconnect_mode())
  , smart_routing_enabled_(
      client.get_client_config().get_network_config().is_smart_routing())
  , is_tpc_aware_client_(
      client.get_client_config().get_tpc_config().is_enabled())
  , client_uuid_(client.random_uuid())
  , authentication_timeout_(
      boost::chrono::milliseconds(heartbeat_.get_heartbeat_timeout().count()))
  , load_balancer_(client.get_client_config().get_load_balancer())
  , wait_strategy_(client.get_client_config()
                     .get_connection_strategy_config()
                     .get_retry_config(),
                   logger_)
  , cluster_id_(boost::uuids::nil_uuid())
  , client_state_(client_state::INITIAL)
  , connect_to_cluster_task_submitted_(false)
  , established_initial_cluster_connection(false)
  , use_public_address_(
      address_provider_->is_default_provider() &&
      client.get_client_config().get_network_config().use_public_address())
{
    config::client_network_config& networkConfig =
      client.get_client_config().get_network_config();
    auto connTimeout = networkConfig.get_connection_timeout();
    if (connTimeout.count() > 0) {
        connection_timeout_millis_ = std::chrono::milliseconds(connTimeout);
    }

    client_properties& clientProperties = client.get_client_properties();
    shuffle_member_list_ =
      clientProperties.get_boolean(clientProperties.get_shuffle_member_list());
}

bool
ClientConnectionManagerImpl::start()
{
    bool expected = false;
    if (!alive_.compare_exchange_strong(expected, true)) {
        return false;
    }

    io_context_.reset(new boost::asio::io_context);
    io_resolver_.reset(
      new boost::asio::ip::tcp::resolver(io_context_->get_executor()));
    socket_factory_.reset(new internal::socket::SocketFactory(
      client_, *io_context_, *io_resolver_));
    io_guard_.reset(new boost::asio::io_context::work(*io_context_));

    if (!socket_factory_->start()) {
        return false;
    }

    socket_interceptor_ = client_.get_client_config().get_socket_interceptor();

    io_thread_ = std::thread([=]() { io_context_->run(); });

    executor_.reset(
      new hazelcast::util::hz_thread_pool(EXECUTOR_CORE_POOL_SIZE));
    connect_to_members_timer_ =
      boost::asio::steady_timer(executor_->get_executor());

    heartbeat_.start();

    connect_to_cluster();
    if (smart_routing_enabled_) {
        schedule_connect_to_all_members();
    }

    load_balancer_.init_(client_.get_cluster());

    return true;
}

void
ClientConnectionManagerImpl::schedule_connect_to_all_members()
{
    if (!client_.get_lifecycle_service().is_running()) {
        return;
    }

    connect_to_members_timer_->expires_from_now(
      boost::asio::chrono::seconds(1));
    connect_to_members_timer_->async_wait([=](boost::system::error_code ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }
        connect_to_all_members();

        schedule_connect_to_all_members();
    });
}

void
ClientConnectionManagerImpl::shutdown()
{
    bool expected = true;
    if (!alive_.compare_exchange_strong(expected, false)) {
        return;
    }

    if (connect_to_members_timer_) {
        connect_to_members_timer_->cancel();
    }

    heartbeat_.shutdown();

    // close connections
    for (auto& connection : active_connections_.values()) {
        // prevent any exceptions
        util::IOUtil::close_resource(connection.get(),
                                     "Hazelcast client is shutting down");
    }

    spi::impl::ClientExecutionServiceImpl::shutdown_thread_pool(
      executor_.get());

    // release the guard so that the io thread can stop gracefully
    io_guard_.reset();
    io_thread_.join();

    connection_listeners_.clear();
    active_connections_.clear();
    active_connection_ids_.clear();
}

std::shared_ptr<Connection>
ClientConnectionManagerImpl::get_or_connect(const member& m)
{
    const auto& uuid = m.get_uuid();
    auto connection = active_connections_.get(uuid);
    if (connection) {
        return connection;
    }

    address addr = translate(m);
    return connect(addr);
}

std::vector<std::shared_ptr<Connection>>
ClientConnectionManagerImpl::get_active_connections()
{
    return active_connections_.values();
}

std::shared_ptr<Connection>
ClientConnectionManagerImpl::get_connection(boost::uuids::uuid uuid)
{
    return active_connections_.get(uuid);
}

authentication_response
ClientConnectionManagerImpl::authenticate_on_cluster(
  std::shared_ptr<Connection>& connection)
{
    auto request =
      encode_authentication_request(client_.get_serialization_service());

    auto clientInvocation =
      spi::impl::ClientInvocation::create(client_, request, "", connection);
    auto f = clientInvocation->invoke_urgent();

    if (f.wait_for(authentication_timeout_) != boost::future_status::ready) {
        BOOST_THROW_EXCEPTION(exception::timeout(
          "ClientConnectionManagerImpl::authenticate",
          (boost::format("Authentication response is "
                         "not received for %1% msecs for %2%") %
           authentication_timeout_.count() % *clientInvocation)
            .str()));
    }

    authentication_response result;

    try {
        auto response = f.get();

        result = authentication_response::from(std::move(response));
    } catch (exception::iexception&) {
        connection->close("Failed to authenticate connection",
                          std::current_exception());
        throw;
    }

    auto authentication_status =
      (protocol::authentication_status)result.status();
    switch (authentication_status) {
        case protocol::AUTHENTICATED: {
            return result;
        }
        case protocol::CREDENTIALS_FAILED: {
            auto e = std::make_exception_ptr(exception::authentication(
              "ClientConnectionManagerImpl::authenticate_on_cluster",
              "Authentication failed. The configured cluster name on the "
              "client (see client_config::set_cluster_name()) does not match "
              "the one configured in the cluster or the credentials set in the "
              "Client security config could not be authenticated"));
            connection->close("Failed to authenticate connection", e);
            std::rethrow_exception(e);
        }
        case protocol::NOT_ALLOWED_IN_CLUSTER: {
            auto e = std::make_exception_ptr(exception::authentication(
              "ClientConnectionManagerImpl::authenticate_on_cluster",
              "Client is not allowed in the cluster"));
            connection->close("Failed to authenticate connection", e);
            std::rethrow_exception(e);
        }
        default: {
            auto e = std::make_exception_ptr(exception::authentication(
              "ClientConnectionManagerImpl::authenticate_on_cluster",
              (boost::format(
                 "Authentication status code not supported. status: %1%") %
               authentication_status)
                .str()));
            connection->close("Failed to authenticate connection", e);
            std::rethrow_exception(e);
        }
    }
}

std::ostream&
operator<<(std::ostream& os, ClientConnectionManagerImpl::client_state state)
{
    using client_state = ClientConnectionManagerImpl::client_state;

    switch (state) {
        case client_state::INITIAL:
            return os << "INITIAL";
        case client_state::CONNECTED_TO_CLUSTER:
            return os << "CONNECTED_TO_CLUSTER";
        case client_state::INITIALIZED_ON_CLUSTER:
            return os << "INITIALIZED_ON_CLUSTER";
        case client_state::DISCONNECTED_FROM_CLUSTER:
            return os << "DISCONNECTED_FROM_CLUSTER";
    }

    return os;
}

protocol::ClientMessage
ClientConnectionManagerImpl::encode_authentication_request(
  serialization::pimpl::SerializationService& ss)
{
    byte serializationVersion = ss.get_version();
    client_config& clientConfig = client_.get_client_config();
    auto cluster_name = clientConfig.get_cluster_name();

    auto credential = clientConfig.get_credentials();
    if (!credential) {
        if (is_tpc_aware_client_) {
            return protocol::codec::experimental_authentication_encode(
              cluster_name,
              nullptr,
              nullptr,
              client_uuid_,
              protocol::ClientTypes::CPP,
              serializationVersion,
              HAZELCAST_VERSION,
              client_.get_name(),
              labels_);
        } else {
            return protocol::codec::client_authentication_encode(
              cluster_name,
              nullptr,
              nullptr,
              client_uuid_,
              protocol::ClientTypes::CPP,
              serializationVersion,
              HAZELCAST_VERSION,
              client_.get_name(),
              labels_);
        }
    }

    switch (credential->type()) {
        case security::credentials::credential_type::username_password: {
            auto cr =
              std::static_pointer_cast<security::username_password_credentials>(
                credential);
            if (is_tpc_aware_client_) {
                return protocol::codec::experimental_authentication_encode(
                  cluster_name,
                  &cr->name(),
                  &cr->password(),
                  client_uuid_,
                  protocol::ClientTypes::CPP,
                  serializationVersion,
                  HAZELCAST_VERSION,
                  client_.get_name(),
                  labels_);
            } else {
                return protocol::codec::client_authentication_encode(
                  cluster_name,
                  &cr->name(),
                  &cr->password(),
                  client_uuid_,
                  protocol::ClientTypes::CPP,
                  serializationVersion,
                  HAZELCAST_VERSION,
                  client_.get_name(),
                  labels_);
            }
        }
        case security::credentials::credential_type::token: {
            if (is_tpc_aware_client_) {
                auto cr = std::static_pointer_cast<security::token_credentials>(
                  credential);
                return protocol::codec::
                  experimental_authenticationcustom_encode(
                    cluster_name,
                    cr->token(),
                    client_uuid_,
                    protocol::ClientTypes::CPP,
                    serializationVersion,
                    HAZELCAST_VERSION,
                    client_.get_name(),
                    labels_);
            } else {
                auto cr = std::static_pointer_cast<security::token_credentials>(
                  credential);
                return protocol::codec::client_authenticationcustom_encode(
                  cluster_name,
                  cr->token(),
                  client_uuid_,
                  protocol::ClientTypes::CPP,
                  serializationVersion,
                  HAZELCAST_VERSION,
                  client_.get_name(),
                  labels_);
            }
        }
    }
    assert(0);
    return protocol::ClientMessage();
}

void
ClientConnectionManagerImpl::fire_connection_added_event(
  const std::shared_ptr<Connection>& connection)
{
    for (const std::shared_ptr<ConnectionListener>& connectionListener :
         connection_listeners_.to_array()) {
        connectionListener->connection_added(connection);
    }
}

void
ClientConnectionManagerImpl::fire_connection_removed_event(
  const std::shared_ptr<Connection>& connection)
{
    for (const auto& listener : connection_listeners_.to_array()) {
        listener->connection_removed(connection);
    }
}

void
ClientConnectionManagerImpl::shutdown_with_external_thread(
  std::weak_ptr<client::impl::hazelcast_client_instance_impl> client_impl)
{
    std::thread([=] {
        std::shared_ptr<client::impl::hazelcast_client_instance_impl>
          clientInstance = client_impl.lock();
        if (!clientInstance ||
            !clientInstance->get_lifecycle_service().is_running()) {
            return;
        }

        try {
            clientInstance->get_lifecycle_service().shutdown();
        } catch (exception::iexception& e) {
            HZ_LOG(*clientInstance->get_logger(),
                   severe,
                   boost::str(boost::format("Exception during client shutdown "
                                            "%1%.clientShutdown-:%2%") %
                              clientInstance->get_name() % e));
        }
    }).detach();
}

void
ClientConnectionManagerImpl::submit_connect_to_cluster_task()
{
    bool expected = false;
    if (!connect_to_cluster_task_submitted_.compare_exchange_strong(expected,
                                                                    true)) {
        return;
    }

    std::weak_ptr<client::impl::hazelcast_client_instance_impl> c =
      client_.get_hazelcast_client_implementation();
    boost::asio::post(executor_->get_executor(), [=]() {
        try {
            do_connect_to_cluster();

            std::lock_guard<std::recursive_mutex> guard(client_state_mutex_);
            connect_to_cluster_task_submitted_ = false;
            if (active_connections_.empty()) {
                HZ_LOG(
                  logger_,
                  finest,
                  boost::str(boost::format("No connection to cluster: %1%") %
                             cluster_id_));

                submit_connect_to_cluster_task();
            }

        } catch (std::exception& e) {
            HZ_LOG(logger_,
                   warning,
                   boost::str(boost::format("Could not connect to any cluster, "
                                            "shutting down the client: %1%") %
                              e.what()));

            shutdown_with_external_thread(c);
        }
    });
}

void
ClientConnectionManagerImpl::connect_to_all_members()
{
    if (!client_.get_lifecycle_service().is_running() ||
        active_connections_.empty()) {
        return;
    }

    for (const auto& m :
         client_.get_client_cluster_service().get_member_list()) {

        {
            std::lock_guard<std::recursive_mutex> guard{ client_state_mutex_ };

            if (client_state_ == client_state::DISCONNECTED_FROM_CLUSTER) {
                // Best effort check to prevent this task from attempting to
                // open a new connection when the client is either switching
                // clusters or is not connected to any of the cluster members.
                // In such occasions, only `do_connect_to_cluster`
                // method should open new connections.
                return;
            }
        }

        if (client_.get_lifecycle_service().is_running() &&
            !get_connection(m.get_uuid()) &&
            connecting_members_.get_or_put_if_absent(m, nullptr).second) {
            // submit a task for this address only if there is no other pending
            // connection attempt for it
            member member_to_connect = m;
            boost::asio::post(
              executor_->get_executor(), [member_to_connect, this]() {
                  try {
                      if (!client_.get_lifecycle_service().is_running()) {
                          return;
                      }
                      if (!get_connection(member_to_connect.get_uuid())) {
                          get_or_connect(member_to_connect);
                      }
                      connecting_members_.remove(member_to_connect);
                  } catch (std::exception&) {
                      connecting_members_.remove(member_to_connect);
                  }
              });
        }
    }
}

void
ClientConnectionManagerImpl::do_connect_to_cluster()
{
    std::unordered_set<address> tried_addresses;
    wait_strategy_.reset();

    do {
        std::unordered_set<address> tried_addresses_per_attempt;
        auto member_list =
          client_.get_client_cluster_service().get_member_list();
        if (shuffle_member_list_) {
            shuffle(member_list);
        }

        // try to connect to a member in the member list first
        for (const auto& m : member_list) {
            check_client_active();
            tried_addresses_per_attempt.insert(m.get_address());
            auto connection = try_connect(m);
            if (connection) {
                return;
            }
        }
        // try to connect to a member given via config(explicit config/discovery
        // mechanisms)
        for (const address& server_address : get_possible_member_addresses()) {
            check_client_active();
            if (!tried_addresses_per_attempt.insert(server_address).second) {
                // if we can not add it means that it is already tried to be
                // connected with the member list
                continue;
            }
            auto connection = try_connect<address>(server_address);
            if (connection) {
                return;
            }
        }
        tried_addresses.insert(tried_addresses_per_attempt.begin(),
                               tried_addresses_per_attempt.end());
        // If the address provider loads no addresses, then the above loop is
        // not entered and the lifecycle check is missing, hence we need to
        // repeat the same check at this point.
        check_client_active();
    } while (wait_strategy_.sleep());

    std::ostringstream out;
    out << "Unable to connect to any address! The following addresses were "
           "tried: { ";
    for (const auto& address : tried_addresses) {
        out << address << " , ";
    }
    out << "}";
    BOOST_THROW_EXCEPTION(exception::illegal_state(
      "ConnectionManager::do_connect_to_cluster", out.str()));
}

std::vector<address>
ClientConnectionManagerImpl::get_possible_member_addresses()
{
    std::vector<address> addresses;
    for (auto&& member :
         client_.get_client_cluster_service().get_member_list()) {
        addresses.emplace_back(std::move(member.get_address()));
    }

    if (shuffle_member_list_) {
        shuffle(addresses);
    }

    std::vector<address> provided_addresses =
      address_provider_->load_addresses();

    if (shuffle_member_list_) {
        shuffle(provided_addresses);
    }

    addresses.insert(
      addresses.end(), provided_addresses.begin(), provided_addresses.end());

    return addresses;
}

void
ClientConnectionManagerImpl::connect_to_cluster()
{
    if (async_start_) {
        submit_connect_to_cluster_task();
    } else {
        do_connect_to_cluster();
    }
}

bool
ClientConnectionManagerImpl::is_alive()
{
    return alive_;
}

void
ClientConnectionManagerImpl::on_connection_close(
  const std::shared_ptr<Connection>& connection)
{
    auto endpoint = connection->get_remote_address();
    auto member_uuid = connection->get_remote_uuid();

    auto socket_remote_address = connection->get_socket().get_remote_endpoint();

    if (!endpoint) {
        HZ_LOG(logger_,
               finest,
               boost::str(boost::format(
                            "Destroying %1% , but it has end-point set to null "
                            "-> not removing it from a connection map") %
                          *connection));
        return;
    }

    {
        std::lock_guard<std::recursive_mutex> guard(client_state_mutex_);

        if (active_connections_.remove(member_uuid, connection)) {
            active_connection_ids_.remove(connection->get_connection_id());

            HZ_LOG(logger_,
                   info,
                   boost::str(
                     boost::format(
                       "Removed connection to endpoint: %1%, connection: %2%") %
                     *endpoint % *connection));

            if (active_connections_.empty()) {
                if (client_state_ == client_state::INITIALIZED_ON_CLUSTER) {
                    fire_life_cycle_event(
                      lifecycle_event::lifecycle_state::CLIENT_DISCONNECTED);
                }

                client_state_ = client_state::DISCONNECTED_FROM_CLUSTER;
                trigger_cluster_reconnection();
            }

            fire_connection_removed_event(connection);
        } else {
            HZ_LOG(
              logger_,
              finest,
              boost::str(boost::format(
                           "Destroying a connection, but there is no mapping "
                           "%1% -> %2% in the connection map.") %
                         endpoint % *connection));
        }
    }
}

void
ClientConnectionManagerImpl::add_connection_listener(
  const std::shared_ptr<ConnectionListener>& connection_listener)
{
    connection_listeners_.add(connection_listener);
}

ClientConnectionManagerImpl::~ClientConnectionManagerImpl()
{
    shutdown();
}

void
ClientConnectionManagerImpl::check_client_active()
{
    if (!client_.get_lifecycle_service().is_running()) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_client_not_active(
          "ClientConnectionManagerImpl::check_client_active",
          "Client is shutdown"));
    }
}

void
ClientConnectionManagerImpl::initialize_client_on_cluster(
  boost::uuids::uuid target_cluster_id)
{
    if (!client_.get_lifecycle_service().is_running()) {
        return;
    }

    try {
        {
            std::lock_guard<std::recursive_mutex> guard(client_state_mutex_);

            if (target_cluster_id != cluster_id_) {
                logger_.log(
                  hazelcast::logger::level::warning,
                  (boost::format("Won't send client state to cluster: %1%"
                                 " Because switched to a new cluster: %2%") %
                   target_cluster_id % cluster_id_)
                    .str());

                return;
            }
        }

        client_.get_hazelcast_client_implementation()->send_state_to_cluster();

        {
            std::lock_guard<std::recursive_mutex> guard(client_state_mutex_);

            if (target_cluster_id == cluster_id_) {
                HZ_LOG(logger_,
                       fine,
                       (boost::format("Client state is sent to cluster: %1%") %
                        target_cluster_id)
                         .str());

                client_state_ = client_state::INITIALIZED_ON_CLUSTER;
                fire_life_cycle_event(lifecycle_event::CLIENT_CONNECTED);
            } else if (logger_.enabled(hazelcast::logger::level::fine)) {
                logger_.log(hazelcast::logger::level::warning,
                            (boost::format("Cannot set client state to %1%"
                                           " because current cluster id: %2%"
                                           " is different than expected cluster"
                                           " id: %3%") %
                             client_state::INITIALIZED_ON_CLUSTER %
                             cluster_id_ % target_cluster_id)
                              .str());
            }
        }
    } catch (const std::exception& e) {
        auto cluster_name = client_.get_client_config().get_cluster_name();

        logger_.log(
          hazelcast::logger::level::warning,
          (boost::format("Failure during sending state to the cluster. %1%") %
           e.what())
            .str());

        {
            std::lock_guard<std::recursive_mutex> guard(client_state_mutex_);

            if (cluster_id_ == target_cluster_id) {
                if (logger_.enabled(hazelcast::logger::level::fine)) {
                    logger_.log(
                      hazelcast::logger::level::warning,
                      (boost::format(
                         "Retrying sending to the cluster: %1%, name: %2%") %
                       target_cluster_id % cluster_name)
                        .str());
                }

                auto self = shared_from_this();

                boost::asio::post(
                  executor_->get_executor(), [self, target_cluster_id]() {
                      self->initialize_client_on_cluster(target_cluster_id);
                  });
            }
        }
    }
}

std::shared_ptr<Connection>
ClientConnectionManagerImpl::on_authenticated(
  const std::shared_ptr<Connection>& connection,
  const authentication_response& response)
{
    {
        std::lock_guard<std::recursive_mutex> guard(client_state_mutex_);
        check_partition_count(response.partition_count());
        connection->set_connected_server_version(
          response.server_hazelcast_version());
        connection->set_remote_address(response.addr());
        connection->set_remote_uuid(response.member_uuid());

        if (is_tpc_aware_client_ && !response.tpc_ports().empty()) {
            connect_to_tpc_ports(connection, response.tpc_ports());
        }

        auto existing_connection =
          active_connections_.get(response.member_uuid());
        if (existing_connection) {
            connection->close(
              (boost::format(
                 "Duplicate connection to same member with uuid : %1%") %
               boost::uuids::to_string(response.member_uuid()))
                .str());
            return existing_connection;
        }

        auto new_cluster_id = response.cluster_id();
        boost::uuids::uuid current_cluster_id = cluster_id_;

        HZ_LOG(logger_,
               finest,
               boost::str(boost::format(
                            "Checking the cluster: %1%, current cluster: %2%") %
                          new_cluster_id % current_cluster_id));

        auto cluster_id_changed = !current_cluster_id.is_nil() &&
                                  !(new_cluster_id == current_cluster_id);
        if (cluster_id_changed) {
            HZ_LOG(
              logger_,
              warning,
              boost::str(
                boost::format(
                  "Switching from current cluster: %1%  to new cluster: %2%") %
                current_cluster_id % new_cluster_id));
            client_.get_hazelcast_client_implementation()->on_cluster_restart();
        }

        auto connections_empty = active_connections_.empty();
        active_connection_ids_.put(connection->get_connection_id(), connection);
        active_connections_.put(response.member_uuid(), connection);
        if (connections_empty) {
            // The first connection that opens a connection to the new cluster
            // should set `clusterId`. This one will initiate
            // `initializeClientOnCluster` if necessary.
            cluster_id_ = new_cluster_id;

            if (established_initial_cluster_connection) {
                // In split brain, the client might connect to the one half
                // of the cluster, and then later might reconnect to the
                // other half, after the half it was connected to is
                // completely dead. Since the cluster id is preserved in
                // split brain scenarios, it is impossible to distinguish
                // reconnection to the same cluster vs reconnection to the
                // other half of the split brain. However, in the latter,
                // we might need to send some state to the other half of
                // the split brain (like Compact schemas or user code
                // deployment classes). That forces us to send the client
                // state to the cluster after the first cluster connection,
                // regardless the cluster id is changed or not.
                client_state_ = client_state::CONNECTED_TO_CLUSTER;
                auto self = shared_from_this();
                boost::asio::post(
                  executor_->get_executor(), [self, new_cluster_id]() {
                      self->initialize_client_on_cluster(new_cluster_id);
                  });
            } else {
                established_initial_cluster_connection = true;
                client_state_ = client_state::INITIALIZED_ON_CLUSTER;

                fire_life_cycle_event(
                  lifecycle_event::lifecycle_state::CLIENT_CONNECTED);
            }
        }

        auto local_address = connection->get_local_socket_address();
        if (local_address) {
            HZ_LOG(
              logger_,
              info,
              boost::str(
                boost::format(
                  "Authenticated with server %1%:%2%, server version: %3%, "
                  "local address: %4%. %5%") %
                response.addr() % response.member_uuid() %
                response.server_hazelcast_version() % *local_address %
                *connection));
        } else {
            HZ_LOG(
              logger_,
              info,
              boost::str(
                boost::format(
                  "Authenticated with server %1%:%2%, server version: %3%, "
                  "no local address: (connection disconnected ?). %4%") %
                response.addr() % response.member_uuid() %
                response.server_hazelcast_version() % *connection));
        }

        fire_connection_added_event(connection);
    }

    // It could happen that this connection is already closed and
    // on_connection_close() is called even before the synchronized block
    // above is executed. In this case, now we have a closed but registered
    // connection. We do a final check here to remove this connection
    // if needed.
    if (!connection->is_alive()) {
        on_connection_close(connection);
        return nullptr;
    }

    // If the client is shutdown in parallel, we need to close this new
    // connection.
    if (!client_.get_lifecycle_service().is_running()) {
        connection->close("Client is shutdown");
    }

    return connection;
}

void
ClientConnectionManagerImpl::connect_to_tpc_ports(
  std::shared_ptr<Connection> conn,
  std::vector<int> ports)
{
    auto connector =
      std::make_shared<tpc_channel_connector>(client_uuid_,
                                              conn,
                                              move(ports),
                                              *executor_,
                                              *socket_factory_,
                                              logger_,
                                              connection_timeout_millis_);
    connector->initiate();
}

void
ClientConnectionManagerImpl::fire_life_cycle_event(
  lifecycle_event::lifecycle_state state)
{
    client_.get_lifecycle_service().fire_lifecycle_event(state);
}

void
ClientConnectionManagerImpl::check_partition_count(int32_t new_partition_count)
{
    auto& partition_service =
      static_cast<spi::impl::ClientPartitionServiceImpl&>(
        client_.get_partition_service());
    if (!partition_service.check_and_set_partition_count(new_partition_count)) {
        BOOST_THROW_EXCEPTION(exception::client_not_allowed_in_cluster(
          "ClientConnectionManagerImpl::check_partition_count",
          (boost::format(
             "Client can not work with this cluster because it has a different "
             "partition count. "
             "Expected partition count: %1%, Member partition count: %2%") %
           partition_service.get_partition_count() % new_partition_count)
            .str()));
    }
}

void
ClientConnectionManagerImpl::trigger_cluster_reconnection()
{
    if (reconnect_mode_ ==
        config::client_connection_strategy_config::reconnect_mode::OFF) {
        HZ_LOG(
          logger_, info, "RECONNECT MODE is off. Shutting down the client.");
        shutdown_with_external_thread(
          client_.get_hazelcast_client_implementation());
        return;
    }

    if (client_.get_lifecycle_service().is_running()) {
        submit_connect_to_cluster_task();
    }
}

std::shared_ptr<Connection>
ClientConnectionManagerImpl::get_random_connection()
{
    if (smart_routing_enabled_) {
        auto member = load_balancer_.next_(client_.get_cluster());
        if (!member) {
            return nullptr;
        }
        auto connection = get_connection(member->get_uuid());
        if (connection) {
            return connection;
        }
    }

    auto connections = active_connections_.values();
    if (connections.empty()) {
        return nullptr;
    }

    return connections[0];
}

boost::uuids::uuid
ClientConnectionManagerImpl::get_client_uuid() const
{
    return client_uuid_;
}

void
ClientConnectionManagerImpl::check_invocation_allowed()
{
    client_state state = client_state_;
    if (state == client_state::INITIALIZED_ON_CLUSTER &&
        active_connections_.size() > 0) {
        return;
    }

    if (state == client_state::INITIAL) {
        if (async_start_) {
            BOOST_THROW_EXCEPTION(exception::hazelcast_client_offline(
              "ClientConnectionManagerImpl::check_invocation_allowed",
              "No connection found to cluster and async start is configured."));
        } else {
            BOOST_THROW_EXCEPTION(exception::io(
              "No connection found to cluster since the client is starting."));
        }
    } else if (reconnect_mode_ == config::client_connection_strategy_config::
                                    reconnect_mode::ASYNC) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_client_offline(
          "ClientConnectionManagerImpl::check_invocation_allowed",
          "No connection found to cluster and reconnect mode is async."));
    } else {
        BOOST_THROW_EXCEPTION(
          exception::io("ClientConnectionManagerImpl::check_invocation_allowed",
                        "No connection found to cluster."));
    }
}

bool
ClientConnectionManagerImpl::client_initialized_on_cluster() const
{
    std::lock_guard<std::recursive_mutex> guard{ client_state_mutex_ };

    return client_state_ == client_state::INITIALIZED_ON_CLUSTER;
}

void
ClientConnectionManagerImpl::connect_to_all_cluster_members()
{
    if (!smart_routing_enabled_) {
        return;
    }

    for (const auto& member :
         client_.get_client_cluster_service().get_member_list()) {

        try {
            get_or_connect(member);
        } catch (std::exception&) {
            // ignore
        }
    }
}

void
ClientConnectionManagerImpl::notify_backup(int64_t call_id)
{
    struct correlation_id
    {
        int32_t connnection_id;
        int32_t call_id;
    };
    union
    {
        int64_t id;
        correlation_id composed_id;
    } c_id_union;
    c_id_union.id = call_id;
    auto connection_id = c_id_union.composed_id.connnection_id;
    auto connection = active_connection_ids_.get(connection_id);
    if (!connection) {
        return;
    }
    boost::asio::post(connection->get_executor(), [=]() {
        auto invocation_it = connection->invocations.find(call_id);
        if (invocation_it != connection->invocations.end()) {
            invocation_it->second->notify_backup();
        }
    });
}

std::shared_ptr<Connection>
ClientConnectionManagerImpl::connect(const address& addr)
{
    HZ_LOG(logger_,
           info,
           boost::str(boost::format("Trying to connect to %1%.") % addr));

    auto connection = std::make_shared<Connection>(addr,
                                                   client_,
                                                   ++connection_id_gen_,
                                                   *socket_factory_,
                                                   *this,
                                                   connection_timeout_millis_);
    connection->connect();

    // call the interceptor from user thread
    socket_interceptor_.connect_(connection->get_socket());

    auto result = authenticate_on_cluster(connection);

    return on_authenticated(connection, result);
}

address
ClientConnectionManagerImpl::translate(const member& m)
{
    if (use_public_address_) {
        auto public_addr_it = m.address_map().find(PUBLIC_ENDPOINT_QUALIFIER);
        if (public_addr_it != m.address_map().end()) {
            return public_addr_it->second;
        }
        return m.get_address();
    }

    try {
        boost::optional<address> addr =
          address_provider_->translate(m.get_address());

        if (!addr) {
            throw exception::hazelcast_(boost::str(
              boost::format("Address Provider could not translate %1%") % m));
        }

        return *addr;
    } catch (const exception::hazelcast_&) {
        logger_.log(
          logger::level::warning,
          boost::str(boost::format("Address Provider could not translate %1%") %
                     m));

        throw;
    }
}

std::shared_ptr<connection::Connection>
ClientConnectionManagerImpl::connection_for_sql(
  std::function<boost::optional<member>()> member_of_large_same_version_group,
  std::function<boost::optional<member>(boost::uuids::uuid)> get_cluster_member)
{
    if (smart_routing_enabled_) {
        // There might be a race - the chosen member might be just connected or
        // disconnected - try a couple of times, the
        // memberOfLargerSameVersionGroup returns a random connection, we might
        // be lucky...
        for (int i = 0; i < SQL_CONNECTION_RANDOM_ATTEMPTS; i++) {
            auto member = member_of_large_same_version_group();
            if (!member) {
                break;
            }
            auto connection = active_connections_.get(member->get_uuid());
            if (connection) {
                return connection;
            }
        }
    }

    // Otherwise iterate over connections and return the first one that's not to
    // a lite member
    std::shared_ptr<connection::Connection> first_connection;
    for (const auto& connection_entry : active_connections_.entry_set()) {
        if (!first_connection) {
            first_connection = connection_entry.second;
        }
        const auto& member_id = connection_entry.first;
        auto member = get_cluster_member(member_id);
        if (!member || member->is_lite_member()) {
            continue;
        }
        return connection_entry.second;
    }

    // Failed to get a connection to a data member
    return first_connection;
}

ReadHandler::ReadHandler(Connection& connection, size_t buffer_size) noexcept
  : buffer(new char[buffer_size])
  , byte_buffer(buffer, buffer_size)
  , builder_(connection)
{}

ReadHandler::~ReadHandler()
{
    delete[] buffer;
}

void
ReadHandler::handle()
{
    {
        std::lock_guard<std::mutex> lock{ read_time_mtx_ };
        last_read_time_ = std::chrono::steady_clock::now();
    }

    if (byte_buffer.position() == 0)
        return;

    byte_buffer.flip();

    // it is important to check the on_data return value since there may be left
    // data less than a message header size, and this may cause an infinite
    // loop.
    while (byte_buffer.has_remaining() && builder_.on_data(byte_buffer)) {
    }

    if (byte_buffer.has_remaining()) {
        byte_buffer.compact();
    } else {
        byte_buffer.clear();
    }
}

std::chrono::steady_clock::time_point
ReadHandler::get_last_read_time() const
{
    std::lock_guard<std::mutex> lock{ read_time_mtx_ };
    return last_read_time_;
}

bool
AddressProvider::is_default_provider()
{
    return false;
}

Connection::Connection(
  const address& address,
  spi::ClientContext& client_context,
  int connection_id, // NOLINT(cppcoreguidelines-pro-type-member-init)
  internal::socket::SocketFactory& socket_factory,
  ClientConnectionManagerImpl& client_connection_manager,
  std::chrono::milliseconds& connect_timeout_in_millis)
  : start_time_(std::chrono::system_clock::now())
  , closed_time_duration_()
  , client_context_(client_context)
  , invocation_service_(client_context.get_invocation_service())
  , connection_id_(connection_id)
  , remote_uuid_(boost::uuids::nil_uuid())
  , tpc_channels_initialized_{ false }
  , logger_(client_context.get_logger())
  , alive_(true)
  , socket_strand_(socket_factory.io_ctx())
{
    socket_ = socket_factory.create(address, connect_timeout_in_millis, *this);
}

Connection::~Connection() = default;

void
Connection::connect()
{
    socket_->connect(shared_from_this());
    backup_timer_.reset(
      new boost::asio::steady_timer(get_executor().context()));
    auto backupTimeout =
      static_cast<spi::impl::ClientInvocationServiceImpl&>(invocation_service_)
        .get_backup_timeout();
    auto this_connection = shared_from_this();
    schedule_periodic_backup_cleanup(backupTimeout, this_connection);
}

void
Connection::schedule_periodic_backup_cleanup(
  std::chrono::milliseconds backup_timeout,
  std::shared_ptr<Connection> this_connection)
{
    if (!alive_) {
        return;
    }

    backup_timer_->expires_from_now(backup_timeout);
    backup_timer_->async_wait(
      get_executor().wrap([=](boost::system::error_code ec) {
          if (ec) {
              return;
          }
          for (const auto& it : this_connection->invocations) {
              it.second->detect_and_handle_backup_timeout(backup_timeout);
          }

          schedule_periodic_backup_cleanup(backup_timeout, this_connection);
      }));
}

void
Connection::close()
{
    close("");
}

void
Connection::close(const std::string& reason)
{
    close(reason, nullptr);
}

void
Connection::close(const std::string& reason, std::exception_ptr cause)
{
    bool expected = true;
    if (!alive_.compare_exchange_strong(expected, false)) {
        return;
    }

    closed_time_duration_.store(
      std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()));

    if (backup_timer_) {
        boost::system::error_code ignored;
        backup_timer_->cancel(ignored);
    }

    close_cause_ = cause;
    close_reason_ = reason;

    log_close();

    try {
        inner_close();
    } catch (exception::iexception& e) {
        HZ_LOG(
          client_context_.get_logger(),
          warning,
          boost::str(boost::format("Exception while closing connection %1%") %
                     e.get_message()));
    }

    auto thisConnection = shared_from_this();
    client_context_.get_connection_manager().on_connection_close(
      thisConnection);

    boost::asio::post(get_executor(), [=]() {
        for (auto& invocationEntry : thisConnection->invocations) {
            invocationEntry.second->notify_exception(std::make_exception_ptr(
              boost::enable_current_exception(exception::target_disconnected(
                "Connection::close", thisConnection->get_close_reason()))));
        }
    });
}

void
Connection::set_tpc_channels(std::vector<std::unique_ptr<socket>> tpc_channels)
{
    tpc_channels_ = std::move(tpc_channels);
    tpc_channels_initialized_ = true;
}

const std::vector<std::unique_ptr<socket>>&
Connection::get_tpc_channels()
{
    static std::vector<std::unique_ptr<socket>> empty_list;

    if (!tpc_channels_initialized_) {
        return empty_list;
    }

    return tpc_channels_;
}

void
Connection::write(
  const std::shared_ptr<spi::impl::ClientInvocation>& client_invocation)
{
    if (!tpc_channels_initialized_ || tpc_channels_.empty()) {
        socket_->async_write(shared_from_this(), client_invocation);

        return;
    }

    int partition_id =
      client_invocation->get_client_message()->get_partition_id();

    if (partition_id < 0) {
        socket_->async_write(shared_from_this(), client_invocation);

        return;
    }

    int channel_index = partition_id % tpc_channels_.size();

    return tpc_channels_[channel_index]->async_write(shared_from_this(),
                                                     client_invocation);
}

const boost::optional<address>&
Connection::get_remote_address() const
{
    return remote_address_;
}

void
Connection::set_remote_address(boost::optional<address> endpoint)
{
    this->remote_address_ = std::move(endpoint);
}

void
Connection::handle_client_message(
  const std::shared_ptr<protocol::ClientMessage>& message)
{
    auto correlationId = message->get_correlation_id();
    auto invocationIterator = invocations.find(correlationId);
    if (invocationIterator == invocations.end()) {
        HZ_LOG(logger_,
               warning,
               boost::str(boost::format("No invocation for callId:  %1%. "
                                        "Dropping this message: %2%") %
                          correlationId % *message));
        return;
    }
    auto invocation = invocationIterator->second;
    auto flags = message->get_header_flags();
    if (message->is_flag_set(flags,
                             protocol::ClientMessage::BACKUP_EVENT_FLAG)) {
        message->rd_ptr(protocol::ClientMessage::EVENT_HEADER_LEN);
        correlationId = message->get<int64_t>();
        client_context_.get_connection_manager().notify_backup(correlationId);
    } else if (message->is_flag_set(flags,
                                    protocol::ClientMessage::IS_EVENT_FLAG)) {
        client_context_.get_client_listener_service().handle_client_message(
          invocation, message);
    } else {
        invocation_service_.handle_client_message(invocation, message);
    }
}

int32_t
Connection::get_connection_id() const
{
    return connection_id_;
}

bool
Connection::is_alive() const
{
    return alive_;
}

const std::string&
Connection::get_close_reason() const
{
    return close_reason_;
}

void
Connection::log_close()
{
    std::ostringstream message;
    message << *this << " closed. Reason: ";
    if (!close_reason_.empty()) {
        message << close_reason_;
    } else if (close_cause_) {
        try {
            std::rethrow_exception(close_cause_);
        } catch (exception::iexception& ie) {
            message << ie.get_source() << "[" + ie.get_message() << "]";
        }
    } else {
        message << "Socket explicitly closed";
    }

    if (client_context_.get_lifecycle_service().is_running()) {
        if (!close_cause_) {
            HZ_LOG(logger_, info, message.str());
        } else {
            try {
                std::rethrow_exception(close_cause_);
            } catch (exception::iexception& ie) {
                HZ_LOG(
                  logger_,
                  warning,
                  boost::str(boost::format("%1% %2%") % message.str() % ie));
            }
        }
    } else {
        HZ_LOG(
          logger_, finest, message.str() + [this]() -> std::string {
              if (close_cause_) {
                  try {
                      std::rethrow_exception(close_cause_);
                  } catch (exception::iexception& ie) {
                      return ie.what();
                  }
              }
              return "";
          }());
    }
}

bool
Connection::operator==(const Connection& rhs) const
{
    return connection_id_ == rhs.connection_id_;
}

bool
Connection::operator!=(const Connection& rhs) const
{
    return !(rhs == *this);
}

const std::string&
Connection::get_connected_server_version_string() const
{
    return connected_server_version_string_;
}

void
Connection::set_connected_server_version(const std::string& connected_server)
{
    Connection::connected_server_version_string_ = connected_server;
}

boost::optional<address>
Connection::get_local_socket_address() const
{
    return socket_->local_socket_address();
}

std::chrono::steady_clock::time_point
Connection::last_read_time() const
{
    return socket_->last_read_time();
}

void
Connection::inner_close()
{
    auto this_connection = shared_from_this();

    if (tpc_channels_initialized_) {
        boost::asio::post(get_executor(), [this_connection]() {
            for (auto& channel : this_connection->get_tpc_channels())
                channel->close();
        });
    }

    if (!socket_) {
        return;
    }

    boost::asio::post(get_executor(),
                      [=]() { this_connection->socket_->close(); });
}

std::ostream&
operator<<(std::ostream& os, const Connection& connection)
{
    os << "Connection{"
       << "alive=" << connection.is_alive()
       << ", connection id=" << connection.get_connection_id()
       << ", remote endpoint=";
    if (connection.get_remote_address()) {
        os << *connection.get_remote_address();
    } else {
        os << "null";
    }
    os << ", last_read_time="
       << util::StringUtil::time_to_string(connection.last_read_time())
       << ", last_write_time="
       << util::StringUtil::time_to_string(connection.last_write_time())
       << ", closedTime="
       << util::StringUtil::time_to_string(
            std::chrono::steady_clock::time_point(
              std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                connection.closed_time_duration_.load())))
       << ", connected server version="
       << connection.connected_server_version_string_ << '}';

    return os;
}

bool
Connection::operator<(const Connection& rhs) const
{
    return connection_id_ < rhs.connection_id_;
}

std::chrono::system_clock::time_point
Connection::get_start_time() const
{
    return start_time_;
}

socket&
Connection::get_socket()
{
    return *socket_;
}

void
Connection::deregister_invocation(int64_t call_id)
{
    invocations.erase(call_id);
}

boost::uuids::uuid
Connection::get_remote_uuid() const
{
    return remote_uuid_;
}

void
Connection::set_remote_uuid(boost::uuids::uuid remote_uuid)
{
    remote_uuid_ = remote_uuid;
}

std::chrono::steady_clock::time_point
Connection::last_write_time() const
{
    return socket_->last_write_time();
}

boost::asio::io_context::strand&
Connection::get_executor()
{
    return socket_strand_;
}

tpc_channel_connector::tpc_channel_connector(
  boost::uuids::uuid client_uuid,
  std::shared_ptr<connection::Connection> conn,
  std::vector<int> ports,
  hazelcast::util::hz_thread_pool& executor,
  internal::socket::SocketFactory& channel_creator,
  logger& l,
  std::chrono::milliseconds connection_timeout_millis)
  : client_uuid_{ client_uuid }
  , connection_{ move(conn) }
  , tpc_ports_{ move(ports) }
  , executor_{ executor }
  , channel_creator_{ channel_creator }
  , logger_{ l }
  , remaining_{ int(tpc_ports_.size()) }
  , failed_{ false }
  , connection_timeout_millis_{ connection_timeout_millis }
{
    generate_n(back_inserter(tpc_channels_), tpc_ports_.size(), []() {
        return nullptr;
    });
}

void
tpc_channel_connector::initiate()
{
    std::vector<std::string> ports_str_vec;
    ports_str_vec.reserve(tpc_ports_.size());

    transform(begin(tpc_ports_),
              end(tpc_ports_),
              back_inserter(ports_str_vec),
              [](int port) { return std::to_string(port); });

    HZ_LOG(logger_,
           info,
           boost::str(boost::format("Initiating connection attempts to TPC "
                                    "channels running on ports %1% for %2%") %
                      boost::algorithm::join(ports_str_vec, " ") %
                      *connection_));

    std::string host = connection_->get_remote_address()->get_host();
    auto self = shared_from_this();

    int i = 0;
    for (int port : tpc_ports_) {
        int index = i++;
        boost::asio::post(
          executor_.get_executor(),
          [self, host, port, index]() { self->connect(host, port, index); });
    }
}

void
tpc_channel_connector::connect(std::string host, int port, int index)
{
    if (connection_failed()) {
        HZ_LOG(logger_,
               warning,
               boost::str(boost::format(
                            R"(The connection to TPC channel on port %1%
                       for %2% will not be made as either the connection
                       or one of the TPC channel connections has failed.)") %
                          port % *connection_));

        return;
    }

    HZ_LOG(
      logger_,
      info,
      boost::str(
        boost::format("Trying to connect to TPC channel on port %1% for %2%") %
        port % *connection_));

    std::unique_ptr<socket> channel;
    try {
        address addr{ host, port };

        channel = channel_creator_.create(
          addr, connection_timeout_millis_, *connection_);
        channel->connect(connection_);
        write_authentication_bytes(*channel);
        on_successful_channel_connection(move(channel), index);
    } catch (const std::exception& e) {
        HZ_LOG(
          logger_,
          warning,
          boost::str(boost::format("Exception during the connection to attempt "
                                   "to TPC channel on port %1% for %2%: %3%") %
                     port % *connection_ % e.what()));

        on_failure(move(channel));
    }
}

bool
tpc_channel_connector::connection_failed() const
{
    return failed_ || !connection_->is_alive();
}

void
tpc_channel_connector::write_authentication_bytes(socket& channel)
{
    protocol::ClientMessage authentication_message{
        protocol::ClientMessage::REQUEST_HEADER_LEN
    };

    auto frame = authentication_message.begin_frame();
    frame.frame_len = protocol::ClientMessage::UUID_SIZE +
                      protocol::ClientMessage::SIZE_OF_FRAME_LENGTH_AND_FLAGS;
    frame.flags = uint64_t(protocol::ClientMessage::UNFRAGMENTED_MESSAGE |
                           protocol::ClientMessage::IS_FINAL_FLAG);
    auto offset = authentication_message.get_buffer().front().size();
    authentication_message.set(frame);
    authentication_message.set(client_uuid_);

    const auto& buffer = authentication_message.get_buffer().front();

    std::vector<byte> buffer_without_header{ buffer.begin() + offset,
                                             buffer.end() };

    if (!channel.write(move(buffer_without_header))) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_{
          boost::str(boost::format("Cannot write authentication bytes to the "
                                   "TPC channel %1% for %2%") %
                     channel % *connection_) });
    }
}

void
tpc_channel_connector::on_successful_channel_connection(
  std::unique_ptr<socket> channel,
  int index)
{
    {
        std::lock_guard<std::recursive_mutex> lock{ mtx_ };

        if (connection_failed()) {
            // It might be the case that the connection or any
            // of the channels are failed after this channel
            // is established. We need to close this one as well
            // to not leak any channels.

            HZ_LOG(
              logger_,
              warning,
              boost::str(boost::format("Closing the TPC channel %1% for %2% as "
                                       "one of the connections is failed.") %
                         *channel % *connection_));
            on_failure(move(channel));
            return;
        }

        tpc_channels_[index] = move(channel);
    }

    HZ_LOG(logger_,
           info,
           boost::str(boost::format(
                        "Successfully connected to TPC channel %1% for %2%") %
                      *tpc_channels_[index] % *connection_));

    if (remaining_.fetch_sub(1) == 1) {
        connection_->set_tpc_channels(move(tpc_channels_));

        // If the connection is alive at this point, but
        // closes afterward, the channels will be cleaned up
        // properly in the connection's close method, because
        // we have already written the channels.

        // If the connection is not alive at this point, the channels
        // might or might not be closed, depending on the order of the
        // close and setTpcChannels calls. We will close channels
        // if the connection is not alive here, just in case, as it is
        // OK to call close on already closed channels.
        if (!connection_->is_alive()) {
            HZ_LOG(
              logger_,
              warning,
              boost::str(boost::format("Closing all TPC channel connections "
                                       "for %1% as the connection is closed.") %
                         *connection_));

            close_all_channels();
        } else {
            HZ_LOG(
              logger_,
              info,
              boost::str(
                boost::format(
                  "All TPC channel connections are established for the %1%") %
                *connection_));
        }
    }
}

void
tpc_channel_connector::on_failure(std::unique_ptr<socket> channel)
{
    std::lock_guard<std::recursive_mutex> lock{ mtx_ };

    close_channel(move(channel));

    if (failed_) {
        return;
    }

    failed_ = true;
    close_all_channels();
    HZ_LOG(logger_,
           warning,
           boost::str(boost::format(
                        R"(
                    TPC channel establishments for the %1% have failed.
                    The client will not be using the TPC channels to route partition specific invocations,
                    and fallback to the smart routing mode for this connection. Check the firewall settings
                    to make sure the TPC channels are accessible from the client.
                )") % *connection_));
}

void
tpc_channel_connector::close_channel(std::unique_ptr<socket> channel)
{
    if (!channel) {
        return;
    }

    try {
        channel->close();
    } catch (const std::exception& e) {
        HZ_LOG(
          logger_,
          warning,
          boost::str(boost::format("Exception while closing TPC channel %1%") %
                     e.what()));
    }
}

void
tpc_channel_connector::close_all_channels()
{
    for (std::unique_ptr<socket>& channel : tpc_channels_) {
        close_channel(move(channel));
    }
}

HeartbeatManager::HeartbeatManager(
  spi::ClientContext& client,
  ClientConnectionManagerImpl& connection_manager)
  : client_(client)
  , client_connection_manager_(connection_manager)
  , logger_(client.get_logger())
{
    client_properties& clientProperties = client.get_client_properties();
    auto timeout_millis =
      clientProperties.get_long(clientProperties.get_heartbeat_timeout());
    heartbeat_timeout_ = std::chrono::milliseconds(
      timeout_millis > 0
        ? timeout_millis
        : util::IOUtil::to_value<int64_t>(
            client_properties::PROP_HEARTBEAT_TIMEOUT_DEFAULT));

    auto interval_millis =
      clientProperties.get_long(clientProperties.get_heartbeat_interval());
    heartbeat_interval_ = std::chrono::milliseconds(
      interval_millis > 0
        ? interval_millis
        : util::IOUtil::to_value<int64_t>(
            client_properties::PROP_HEARTBEAT_INTERVAL_DEFAULT));
}

void
HeartbeatManager::start()
{
    spi::impl::ClientExecutionServiceImpl& clientExecutionService =
      client_.get_client_execution_service();

    timer_ = clientExecutionService.schedule_with_repetition(
      [=]() {
          if (!client_connection_manager_.is_alive()) {
              return;
          }

          for (auto& connection :
               client_connection_manager_.get_active_connections()) {
              check(connection);

              auto now = std::chrono::steady_clock::now();
              for (std::size_t i = 0; i < connection->get_tpc_channels().size();
                   ++i) {
                  check(connection, *connection->get_tpc_channels()[i], i, now);
              }
          }
      },
      heartbeat_interval_,
      heartbeat_interval_);
}

void
HeartbeatManager::check(const std::shared_ptr<Connection>& connection)
{
    if (!connection->is_alive()) {
        return;
    }

    auto now = std::chrono::steady_clock::now();
    if (now - connection->last_read_time() > heartbeat_timeout_) {
        HZ_LOG(logger_,
               warning,
               boost::str(
                 boost::format("Heartbeat failed over the connection: %1%") %
                 *connection));
        on_heartbeat_stopped(connection, "Heartbeat timed out");
        return;
    }

    if (now - connection->last_write_time() > heartbeat_interval_) {
        auto request = protocol::codec::client_ping_encode();
        std::shared_ptr<spi::impl::ClientInvocation> clientInvocation =
          spi::impl::ClientInvocation::create(client_, request, "", connection);
        clientInvocation->invoke_urgent();
    }
}

void
HeartbeatManager::check(const std::shared_ptr<Connection>& connection,
                        socket& tpc_channel,
                        int channel_index,
                        std::chrono::steady_clock::time_point now)
{
    if (!connection->is_alive()) {
        return;
    }

    if (tpc_channel.is_closed()) {
        return;
    }

    auto last_read_time = tpc_channel.last_read_time();

    if (last_read_time != std::chrono::steady_clock::time_point{} &&
        now - last_read_time > heartbeat_timeout_) {
        HZ_LOG(
          logger_,
          warning,
          boost::str(
            boost::format(
              "Heartbeat failed over the TPC channel %1% for connection: %2%") %
            tpc_channel % *connection));

        on_heartbeat_stopped(connection, "Heartbeat timed out");
        return;
    }

    if (now - tpc_channel.last_write_time() > heartbeat_interval_) {
        auto request = protocol::codec::client_ping_encode();
        request.set_partition_id(channel_index);
        std::shared_ptr<spi::impl::ClientInvocation> clientInvocation =
          spi::impl::ClientInvocation::create(client_, request, "", connection);
        clientInvocation->invoke_urgent();
    }
}

void
HeartbeatManager::on_heartbeat_stopped(
  const std::shared_ptr<Connection>& connection,
  const std::string& reason)
{
    connection->close(
      reason,
      std::make_exception_ptr(
        (exception::exception_builder<exception::target_disconnected>(
           "HeartbeatManager::onHeartbeatStopped")
         << "Heartbeat timed out to connection " << *connection)
          .build()));
}

void
HeartbeatManager::shutdown()
{
    if (timer_) {
        boost::system::error_code ignored;
        timer_->cancel(ignored);
    }
}

std::chrono::milliseconds
HeartbeatManager::get_heartbeat_timeout() const
{
    return heartbeat_timeout_;
}

void
wait_strategy::reset()
{
    attempt_ = 0;
    cluster_connect_attempt_begin_ = std::chrono::steady_clock::now();
    current_backoff_millis_ =
      (std::min)(max_backoff_millis_, initial_backoff_millis_);
}

wait_strategy::wait_strategy(
  const config::connection_retry_config& retry_config,
  logger& log)
  : initial_backoff_millis_(retry_config.get_initial_backoff_duration())
  , max_backoff_millis_(retry_config.get_max_backoff_duration())
  , multiplier_(retry_config.get_multiplier())
  , jitter_(retry_config.get_jitter())
  , logger_(log)
  , cluster_connect_timeout_millis_(retry_config.get_cluster_connect_timeout())
{
    if (cluster_connect_timeout_millis_ == std::chrono::milliseconds::max()) {
        cluster_connect_timeout_text_ = "INFINITE";
    } else {
        cluster_connect_timeout_text_ =
          (boost::format("%1% msecs") % cluster_connect_timeout_millis_.count())
            .str();
    }
}

bool
wait_strategy::sleep()
{
    attempt_++;
    using namespace std::chrono;
    auto current_time = steady_clock::now();
    auto time_passed = duration_cast<milliseconds>(
      current_time - cluster_connect_attempt_begin_);
    if (time_passed > cluster_connect_timeout_millis_) {
        HZ_LOG(
          logger_,
          warning,
          (boost::format("Unable to get live cluster connection, cluster "
                         "connect timeout (%1%) is reached. Attempt %2%.") %
           cluster_connect_timeout_text_ % attempt_)
            .str());
        return false;
    }

    // sleep time: current_backoff_millis_(1 +- (jitter * [0, 1]))
    auto actual_sleep_time =
      current_backoff_millis_ + milliseconds(static_cast<milliseconds::rep>(
                                  current_backoff_millis_.count() * jitter_ *
                                  (2.0 * random_(random_generator_) - 1.0)));

    actual_sleep_time = (std::min)(
      actual_sleep_time, cluster_connect_timeout_millis_ - time_passed);

    HZ_LOG(
      logger_,
      warning,
      (boost::format(
         "Unable to get live cluster connection, retry in %1% ms, attempt: %2% "
         ", cluster connect timeout: %3% , max backoff millis: %4%") %
       actual_sleep_time.count() % attempt_ % cluster_connect_timeout_text_ %
       max_backoff_millis_.count())
        .str());

    std::this_thread::sleep_for(actual_sleep_time);

    current_backoff_millis_ =
      (std::min)(milliseconds(static_cast<milliseconds::rep>(
                   current_backoff_millis_.count() * multiplier_)),
                 max_backoff_millis_);
    return true;
}
} // namespace connection

namespace internal {
namespace socket {
SocketFactory::SocketFactory(spi::ClientContext& client_context,
                             boost::asio::io_context& io,
                             boost::asio::ip::tcp::resolver& resolver)
  : client_context_(client_context)
  , io_(io)
  , io_resolver_(resolver)
{}

bool
SocketFactory::start()
{
#ifdef HZ_BUILD_WITH_SSL
    auto& sslConfig =
      client_context_.get_client_config().get_network_config().get_ssl_config();
    if (sslConfig.is_enabled()) {
        if (sslConfig.ssl_context_) {
            ssl_context_ = sslConfig.ssl_context_;
        } else {
            ssl_context_ = std::make_shared<boost::asio::ssl::context>(
              (boost::asio::ssl::context_base::method)sslConfig.get_protocol());

            ssl_context_->set_verify_mode(boost::asio::ssl::verify_peer);
            ssl_context_->set_default_verify_paths();

            const std::vector<std::string>& verifyFiles =
              sslConfig.get_verify_files();
            bool success = true;
            logger& lg = client_context_.get_logger();
            for (const auto& f : verifyFiles) {
                boost::system::error_code ec;
                ssl_context_->load_verify_file(f, ec);
                if (ec) {
                    HZ_LOG(
                      lg,
                      warning,
                      boost::str(
                        boost::format("SocketFactory::start: Failed to load CA "
                                      "verify file at %1% %2%") %
                        f % ec.message()));
                    success = false;
                }
            }

            if (!success) {
                ssl_context_.reset();
                HZ_LOG(lg,
                       warning,
                       "SocketFactory::start: Failed to load one or more "
                       "configured CA verify files (PEM files). Please "
                       "correct the files and retry.");
                return false;
            }
        }

        // set cipher list if the list is set
        const std::string& cipherList = sslConfig.get_cipher_list();
        if (!cipherList.empty()) {
            if (!SSL_CTX_set_cipher_list(ssl_context_->native_handle(),
                                         cipherList.c_str())) {
                logger& lg = client_context_.get_logger();
                HZ_LOG(lg,
                       warning,
                       std::string("SocketFactory::start: Could not load any "
                                   "of the ciphers in the config provided "
                                   "ciphers:") +
                         cipherList);
                return false;
            }
        }
    }
#else
    (void)client_context_;
#endif
    return true;
}

std::unique_ptr<hazelcast::client::socket>
SocketFactory::create(const address& address,
                      std::chrono::milliseconds& connect_timeout_in_millis,
                      connection::Connection& connection)
{
#ifdef HZ_BUILD_WITH_SSL
    if (ssl_context_.get()) {
        return std::unique_ptr<hazelcast::client::socket>(
          new internal::socket::SSLSocket(io_,
                                          *ssl_context_,
                                          address,
                                          client_context_.get_client_config()
                                            .get_network_config()
                                            .get_socket_options(),
                                          connect_timeout_in_millis,
                                          io_resolver_,
                                          connection));
    }
#endif

    return std::unique_ptr<hazelcast::client::socket>(
      new internal::socket::TcpSocket(io_,
                                      address,
                                      client_context_.get_client_config()
                                        .get_network_config()
                                        .get_socket_options(),
                                      connect_timeout_in_millis,
                                      io_resolver_,
                                      connection));
}

boost::asio::io_context&
SocketFactory::io_ctx()
{
    return io_;
}

#ifdef HZ_BUILD_WITH_SSL

SSLSocket::SSLSocket(boost::asio::io_context& io_service,
                     boost::asio::ssl::context& ssl_context,
                     const client::address& address,
                     client::config::socket_options& socket_options,
                     std::chrono::milliseconds& connect_timeout_in_millis,
                     boost::asio::ip::tcp::resolver& resolver,
                     connection::Connection& connection)
  : BaseSocket<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(
      resolver,
      address,
      socket_options,
      io_service,
      connect_timeout_in_millis,
      connection,
      ssl_context)
{}

std::vector<SSLSocket::CipherInfo>
SSLSocket::get_ciphers()
{
    STACK_OF(SSL_CIPHER)* ciphers = SSL_get_ciphers(socket_.native_handle());
    std::vector<CipherInfo> supportedCiphers;
    for (int i = 0; i < sk_SSL_CIPHER_num(ciphers); ++i) {
        struct SSLSocket::CipherInfo info;
        const SSL_CIPHER* cipher = sk_SSL_CIPHER_value(ciphers, i);
        info.name = SSL_CIPHER_get_name(cipher);
        info.number_of_bits = SSL_CIPHER_get_bits(cipher, 0);
        info.version = SSL_CIPHER_get_version(cipher);
        char descBuf[256];
        info.description = SSL_CIPHER_description(cipher, descBuf, 256);
        supportedCiphers.push_back(info);
    }
    return supportedCiphers;
}

void
SSLSocket::post_connect()
{
    socket_.handshake(boost::asio::ssl::stream_base::client);
}

std::ostream&
operator<<(std::ostream& out, const SSLSocket::CipherInfo& info)
{
    out << "Cipher{"
           "Name: "
        << info.name << ", Bits:" << info.number_of_bits
        << ", Version:" << info.version << ", Description:" << info.description
        << "}";

    return out;
}

#endif // HZ_BUILD_WITH_SSL

TcpSocket::TcpSocket(boost::asio::io_context& io,
                     const address& address,
                     client::config::socket_options& socket_options,
                     std::chrono::milliseconds& connect_timeout_in_millis,
                     boost::asio::ip::tcp::resolver& resolver,
                     connection::Connection& connection)
  : BaseSocket<boost::asio::ip::tcp::socket>(resolver,
                                             address,
                                             socket_options,
                                             io,
                                             connect_timeout_in_millis,
                                             connection)
{}

} // namespace socket
} // namespace internal
} // namespace client
} // namespace hazelcast

namespace std {
std::size_t
hash<std::shared_ptr<hazelcast::client::connection::Connection>>::operator()(
  const std::shared_ptr<hazelcast::client::connection::Connection>& conn)
  const noexcept
{
    if (!conn) {
        return 0;
    }
    return std::abs(conn->get_connection_id());
}
} // namespace std
