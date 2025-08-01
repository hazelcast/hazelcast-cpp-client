/*
 * Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.
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

#include <utility>
#include <vector>
#include <functional>

#include <boost/format.hpp>

#include "hazelcast/util/Util.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/hazelcast_client.h"
#include "hazelcast/client/transaction_context.h"
#include "hazelcast/client/cluster.h"
#include "hazelcast/client/spi/lifecycle_service.h"
#include "hazelcast/client/lifecycle_listener.h"
#include "hazelcast/client/exception/protocol_exceptions.h"
#include "hazelcast/client/aws/aws_client.h"
#include "hazelcast/client/spi/impl/discovery/cloud_discovery.h"
#include "hazelcast/client/impl/statistics/Statistics.h"
#include "hazelcast/client/impl/hazelcast_client_instance_impl.h"
#include "hazelcast/client/impl/ClientLockReferenceIdGenerator.h"
#include "hazelcast/client/spi/impl/ClientInvocationServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/spi/impl/sequence/CallIdFactory.h"
#include "hazelcast/client/spi/impl/DefaultAddressProvider.h"
#include "hazelcast/client/spi/impl/discovery/remote_address_provider.h"
#include "hazelcast/client/spi/impl/listener/cluster_view_listener.h"
#include "hazelcast/client/spi/impl/listener/listener_service_impl.h"
#include "hazelcast/client/load_balancer.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/proxy/flake_id_generator_impl.h"
#include "hazelcast/client/map/NearCachedClientMapProxy.h"
#include "hazelcast/logger.h"
#include "hazelcast/client/member_selectors.h"
#include "hazelcast/client/client_properties.h"
#include "hazelcast/client/big_decimal.h"
#include "hazelcast/client/local_time.h"
#include "hazelcast/client/local_date.h"
#include "hazelcast/client/local_date_time.h"
#include "hazelcast/client/offset_date_time.h"
#ifndef HAZELCAST_VERSION
#define HAZELCAST_VERSION "NOT_FOUND"
#endif

namespace hazelcast {
namespace client {
hazelcast_client::hazelcast_client()
  : client_impl_(
      new impl::hazelcast_client_instance_impl(*this, client_config()))
{
    client_impl_->start();
}

hazelcast_client::hazelcast_client(client_config config)
  : client_impl_(
      new impl::hazelcast_client_instance_impl(*this, std::move(config)))
{
    client_impl_->start();
}

const std::string&
hazelcast_client::get_name() const
{
    return client_impl_->get_name();
}

client_config&
hazelcast_client::get_client_config()
{
    return client_impl_->get_client_config();
}

transaction_context
hazelcast_client::new_transaction_context()
{
    return client_impl_->new_transaction_context();
}

transaction_context
hazelcast_client::new_transaction_context(const transaction_options& options)
{
    return client_impl_->new_transaction_context(options);
}

cluster&
hazelcast_client::get_cluster()
{
    return client_impl_->get_cluster();
}

boost::uuids::uuid
hazelcast_client::add_lifecycle_listener(
  lifecycle_listener&& lifecycle_listener)
{
    return client_impl_->add_lifecycle_listener(std::move(lifecycle_listener));
}

bool
hazelcast_client::remove_lifecycle_listener(
  const boost::uuids::uuid& registration_id)
{
    return client_impl_->remove_lifecycle_listener(registration_id);
}

boost::future<void>
hazelcast_client::shutdown()
{
    return boost::async([=]() { client_impl_->shutdown(); });
}

spi::lifecycle_service&
hazelcast_client::get_lifecycle_service()
{
    return client_impl_->get_lifecycle_service();
}

local_endpoint
hazelcast_client::get_local_endpoint() const
{
    return client_impl_->get_local_endpoint();
}

hazelcast_client::~hazelcast_client() = default;

cp::cp_subsystem&
hazelcast_client::get_cp_subsystem()
{
    return client_impl_->get_cp_subsystem();
}

sql::sql_service&
hazelcast_client::get_sql()
{
    return client_impl_->get_sql();
}

const boost::string_view
version()
{
    return HAZELCAST_VERSION;
}

namespace impl {
std::atomic<int32_t> hazelcast_client_instance_impl::CLIENT_ID(0);

hazelcast_client_instance_impl::hazelcast_client_instance_impl(
  hazelcast::client::hazelcast_client& /* client */,
  client_config config)
  : client_config_(std::move(config))
  , client_properties_(client_config_.get_properties())
  , client_context_(*this)
  , schema_service_{ client_context_ }
  , serialization_service_(client_config_.get_serialization_config(),
                           schema_service_)
  , cluster_service_(client_context_)
  , transaction_manager_(client_context_)
  , cluster_(cluster_service_)
  , lifecycle_service_(client_context_,
                       client_config_.get_lifecycle_listeners())
  , proxy_manager_(client_context_)
  , id_(++CLIENT_ID)
  , random_generator_(std::random_device{}())
  , uuid_generator_{ random_generator_ }
  , cp_subsystem_(client_context_)
  , sql_service_(client_context_)
  , proxy_session_manager_(client_context_)
{
    auto& name = client_config_.get_instance_name();
    if (name) {
        instance_name_ = *name;
    } else {
        std::ostringstream out;
        out << "hz.client_" << id_;
        instance_name_ = out.str();
    }

    auto logger_config = client_config_.get_logger_config();
    logger_ = std::make_shared<logger>(instance_name_,
                                       client_config_.get_cluster_name(),
                                       logger_config.level(),
                                       logger_config.handler());

    execution_service_ = init_execution_service();

    initalize_near_cache_manager();

    int32_t maxAllowedConcurrentInvocations = client_properties_.get_integer(
      client_properties_.get_max_concurrent_invocations());
    int64_t backofftimeoutMs = client_properties_.get_long(
      client_properties_.get_backpressure_backoff_timeout_millis());
    bool isBackPressureEnabled = maxAllowedConcurrentInvocations != INT32_MAX;
    call_id_sequence_ =
      spi::impl::sequence::CallIdFactory::new_call_id_sequence(
        isBackPressureEnabled,
        maxAllowedConcurrentInvocations,
        backofftimeoutMs);

    auto address_provider = create_address_provider();

    connection_manager_ =
      std::make_shared<connection::ClientConnectionManagerImpl>(
        client_context_, std::move(address_provider));

    cluster_listener_.reset(
      new spi::impl::listener::cluster_view_listener(client_context_));

    partition_service_.reset(
      new spi::impl::ClientPartitionServiceImpl(client_context_));

    invocation_service_.reset(
      new spi::impl::ClientInvocationServiceImpl(client_context_));

    listener_service_ = init_listener_service();

    proxy_manager_.init();

    lock_reference_id_generator_.reset(
      new impl::ClientLockReferenceIdGenerator());

    statistics_.reset(new statistics::Statistics(client_context_));
}

hazelcast_client_instance_impl::~hazelcast_client_instance_impl()
{
    shutdown();
}

void
hazelcast_client_instance_impl::start()
{
    lifecycle_service_.fire_lifecycle_event(lifecycle_event::STARTING);

    try {
        if (!lifecycle_service_.start()) {
            lifecycle_service_.shutdown();
            BOOST_THROW_EXCEPTION(exception::illegal_state(
              "hazelcast_client", "hazelcast_client could not be started!"));
        }
    } catch (std::exception&) {
        lifecycle_service_.shutdown();
        throw;
    }
}

client_config&
hazelcast_client_instance_impl::get_client_config()
{
    return client_config_;
}

cluster&
hazelcast_client_instance_impl::get_cluster()
{
    return cluster_;
}

boost::uuids::uuid
hazelcast_client_instance_impl::add_lifecycle_listener(
  lifecycle_listener&& lifecycle_listener)
{
    return lifecycle_service_.add_listener(std::move(lifecycle_listener));
}

bool
hazelcast_client_instance_impl::remove_lifecycle_listener(
  const boost::uuids::uuid& registration_id)
{
    return lifecycle_service_.remove_listener(registration_id);
}

void
hazelcast_client_instance_impl::shutdown()
{
    lifecycle_service_.shutdown();
}

transaction_context
hazelcast_client_instance_impl::new_transaction_context()
{
    transaction_options defaultOptions;
    return new_transaction_context(defaultOptions);
}

transaction_context
hazelcast_client_instance_impl::new_transaction_context(
  const transaction_options& options)
{
    return transaction_context(transaction_manager_, options);
}

internal::nearcache::NearCacheManager&
hazelcast_client_instance_impl::get_near_cache_manager()
{
    return *near_cache_manager_;
}

serialization::pimpl::SerializationService&
hazelcast_client_instance_impl::get_serialization_service()
{
    return serialization_service_;
}

const protocol::ClientExceptionFactory&
hazelcast_client_instance_impl::get_exception_factory() const
{
    return exception_factory_;
}

std::shared_ptr<spi::impl::listener::listener_service_impl>
hazelcast_client_instance_impl::init_listener_service()
{
    auto eventThreadCount = client_properties_.get_integer(
      client_properties_.get_event_thread_count());
    return std::make_shared<spi::impl::listener::listener_service_impl>(
      client_context_, eventThreadCount);
}

std::shared_ptr<spi::impl::ClientExecutionServiceImpl>
hazelcast_client_instance_impl::init_execution_service()
{
    return std::make_shared<spi::impl::ClientExecutionServiceImpl>(
      instance_name_,
      client_properties_,
      client_config_.get_executor_pool_size(),
      lifecycle_service_);
}

void
hazelcast_client_instance_impl::on_cluster_restart()
{
    HZ_LOG(*logger_,
           info,
           "Clearing local state of the client, because of a cluster restart");

    near_cache_manager_->clear_all_near_caches();
    // clear the member list
    cluster_service_.clear_member_list();
}

std::unique_ptr<connection::AddressProvider>
hazelcast_client_instance_impl::create_address_provider()
{
    config::client_network_config& networkConfig =
      get_client_config().get_network_config();
    config::client_aws_config& awsConfig = networkConfig.get_aws_config();
    config::cloud_config& cloudConfig = networkConfig.get_cloud_config();

    auto addresses = networkConfig.get_addresses();
    bool addressListProvided = !addresses.empty();
    bool awsDiscoveryEnabled = awsConfig.is_enabled();
    bool cloud_enabled = cloudConfig.enabled;

    check_discovery_configuration_consistency(
      addressListProvided, awsDiscoveryEnabled, cloud_enabled);

    auto connect_timeout = networkConfig.get_connection_timeout();
    if (cloud_enabled) {
        auto cloud_provider =
          std::make_shared<spi::impl::discovery::cloud_discovery>(
            cloudConfig,
            client_properties_.get_string(client_properties_.cloud_base_url()),
            connect_timeout);
        return std::unique_ptr<connection::AddressProvider>(
          new spi::impl::discovery::remote_address_provider(
            [=]() { return cloud_provider->get_addresses(); }, true));
    }

    if (awsDiscoveryEnabled) {
        auto aws_addr_provider = std::make_shared<aws::aws_client>(
          connect_timeout, awsConfig, client_properties_, *logger_);
        return std::unique_ptr<connection::AddressProvider>(
          new spi::impl::discovery::remote_address_provider(
            [=]() { return aws_addr_provider->get_addresses(); },
            !awsConfig.is_inside_aws()));
    }

    return std::unique_ptr<connection::AddressProvider>(
      new spi::impl::DefaultAddressProvider(networkConfig));
}

const std::string&
hazelcast_client_instance_impl::get_name() const
{
    return instance_name_;
}

spi::lifecycle_service&
hazelcast_client_instance_impl::get_lifecycle_service()
{
    return lifecycle_service_;
}

const std::shared_ptr<ClientLockReferenceIdGenerator>&
hazelcast_client_instance_impl::get_lock_reference_id_generator() const
{
    return lock_reference_id_generator_;
}

spi::ProxyManager&
hazelcast_client_instance_impl::get_proxy_manager()
{
    return proxy_manager_;
}

void
hazelcast_client_instance_impl::initalize_near_cache_manager()
{
    near_cache_manager_.reset(new internal::nearcache::NearCacheManager(
      execution_service_, serialization_service_, *logger_));
}

local_endpoint
hazelcast_client_instance_impl::get_local_endpoint() const
{
    return cluster_service_.get_local_client();
}

template<>
boost::shared_future<std::shared_ptr<imap>>
hazelcast_client_instance_impl::get_distributed_object(const std::string& name)
{
    auto nearCacheConfig = client_config_.get_near_cache_config(name);
    if (nearCacheConfig) {
        return proxy_manager_
          .get_or_create_proxy<
            map::NearCachedClientMapProxy<serialization::pimpl::data,
                                          serialization::pimpl::data>>(
            imap::SERVICE_NAME, name)
          .then(
            boost::launch::sync,
            [=](
              boost::shared_future<std::shared_ptr<
                map::NearCachedClientMapProxy<serialization::pimpl::data,
                                              serialization::pimpl::data>>> f) {
                return std::static_pointer_cast<imap>(f.get());
            });
    } else {
        return proxy_manager_.get_or_create_proxy<imap>(imap::SERVICE_NAME,
                                                        name);
    }
}

const std::shared_ptr<logger>&
hazelcast_client_instance_impl::get_logger() const
{
    return logger_;
}

boost::uuids::uuid
hazelcast_client_instance_impl::random_uuid()
{
    std::lock_guard<std::mutex> g(uuid_generator_lock_);
    return uuid_generator_();
}

cp::cp_subsystem&
hazelcast_client_instance_impl::get_cp_subsystem()
{
    return cp_subsystem_;
}

sql::sql_service&
hazelcast_client_instance_impl::get_sql()
{
    return sql_service_;
}

void
hazelcast_client_instance_impl::send_state_to_cluster()
{
    schema_service_.replicate_all_schemas();
}

bool
hazelcast_client_instance_impl::should_check_urgent_invocations() const
{
    return schema_service_.has_any_schemas();
}

void
hazelcast_client_instance_impl::check_discovery_configuration_consistency(
  bool address_list_provided,
  bool aws_enabled,
  bool cloud_enabled)
{
    int count = 0;
    if (address_list_provided)
        count++;
    if (aws_enabled)
        count++;
    if (cloud_enabled)
        count++;
    if (count > 1) {
        BOOST_THROW_EXCEPTION(exception::illegal_state(
          "hazelcast_client_instance_impl::check_discovery_configuration_"
          "consistency",
          (boost::format(
             "Only one discovery method can be enabled at a time. cluster "
             "members given explicitly : %1%, aws discovery: %2%, "
             "hazelcast.cloud enabled : %3%") %
           address_list_provided % aws_enabled % cloud_enabled)
            .str()));
    }
}

BaseEventHandler::~BaseEventHandler() = default;

BaseEventHandler::BaseEventHandler(logger &logger)
  : logger_(logger)
{}

logger &
BaseEventHandler::get_logger() const
{
    return logger_;
}
} // namespace impl

constexpr int address::ID;

address::address()
  : host_("localhost")
  , type_(IPV4)
  , scope_id_(0)
{}

address::address(std::string url, int port)
  : host_(std::move(url))
  , port_(port)
  , type_(IPV4)
  , scope_id_(0)
{}

address::address(std::string hostname, int port, unsigned long scope_id)
  : host_(std::move(hostname))
  , port_(port)
  , type_(IPV6)
  , scope_id_(scope_id)
{}

bool
address::operator==(const address& rhs) const
{
    return rhs.port_ == port_ && rhs.type_ == type_ &&
           0 == rhs.host_.compare(host_);
}

bool
address::operator!=(const address& rhs) const
{
    return !(*this == rhs);
}

int
address::get_port() const
{
    return port_;
}

const std::string&
address::get_host() const
{
    return host_;
}

bool
address::operator<(const address& rhs) const
{
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

bool
address::is_ip_v4() const
{
    return type_ == IPV4;
}

unsigned long
address::get_scope_id() const
{
    return scope_id_;
}

std::string
address::to_string() const
{
    std::ostringstream out;
    out << "Address[" << get_host() << ":" << get_port() << "]";
    return out.str();
}

std::ostream&
operator<<(std::ostream& stream, const address& address)
{
    return stream << address.to_string();
}

bool
operator==(const big_decimal& lhs, const big_decimal& rhs)
{
    return lhs.unscaled == rhs.unscaled && lhs.scale == rhs.scale;
}

bool
operator<(const big_decimal& lhs, const big_decimal& rhs)
{
    if (lhs.scale != rhs.scale) {
        return lhs.scale < rhs.scale;
    }
    return lhs.unscaled < rhs.unscaled;
}

bool
operator==(const local_time& lhs, const local_time& rhs)
{
    return lhs.hours == rhs.hours && lhs.minutes == rhs.minutes &&
           lhs.seconds == rhs.seconds && lhs.nanos == rhs.nanos;
}

/**
 * Glue method to enable boost::combine_has to call std::hash for the types
 * under hazelcast::client namespace.
 */
template<typename T>
std::size_t
hash_value(const T& v)
{
    return std::hash<T>()(v);
}

bool
operator<(const local_time& lhs, const local_time& rhs)
{
    if (lhs.hours < rhs.hours) {
        return true;
    }
    if (rhs.hours < lhs.hours) {
        return false;
    }
    if (lhs.minutes < rhs.minutes) {
        return true;
    }
    if (rhs.minutes < lhs.minutes) {
        return false;
    }
    if (lhs.seconds < rhs.seconds) {
        return true;
    }
    if (rhs.seconds < lhs.seconds) {
        return false;
    }
    return lhs.nanos < rhs.nanos;
}

bool
operator==(const local_date& lhs, const local_date& rhs)
{
    return lhs.year == rhs.year && lhs.month == rhs.month &&
           lhs.day_of_month == rhs.day_of_month;
}

bool
operator<(const local_date& lhs, const local_date& rhs)
{
    if (lhs.year < rhs.year) {
        return true;
    }
    if (rhs.year < lhs.year) {
        return false;
    }
    if (lhs.month < rhs.month) {
        return true;
    }
    if (rhs.month < lhs.month) {
        return false;
    }
    return lhs.day_of_month < rhs.day_of_month;
}

bool
operator==(const local_date_time& lhs, const local_date_time& rhs)
{
    return lhs.date == rhs.date && lhs.time == rhs.time;
}

bool
operator<(const local_date_time& lhs, const local_date_time& rhs)
{
    if (lhs.date < rhs.date) {
        return true;
    }
    if (rhs.date < lhs.date) {
        return false;
    }
    return lhs.time < rhs.time;
}

bool
operator==(const offset_date_time& lhs, const offset_date_time& rhs)
{
    return lhs.date_time == rhs.date_time &&
           lhs.zone_offset_in_seconds == rhs.zone_offset_in_seconds;
}

bool
operator<(const offset_date_time& lhs, const offset_date_time& rhs)
{
    if (lhs.date_time < rhs.date_time) {
        return true;
    }
    if (rhs.date_time < lhs.date_time) {
        return false;
    }
    return lhs.zone_offset_in_seconds < rhs.zone_offset_in_seconds;
}

namespace pimpl {

void
twos_complement(std::vector<int8_t>& a)
{
    // twos complement is calculated via flipping the bits and adding 1
    // flip the bits
    for (auto& item : a) {
        item = ~item;
    }
    // add 1
    int8_t carry = 1;
    for (int i = a.size() - 1; i >= 0; i--) {
        a[i] = a[i] + carry;
        if (a[i] == 0) {
            carry = 1;
        } else {
            break;
        }
    }
}

boost::multiprecision::cpp_int
from_bytes(std::vector<int8_t> v)
{
    boost::multiprecision::cpp_int i;
    bool is_negative = v[0] < 0;
    if (is_negative) {
        twos_complement(v);
    }
    import_bits(i, v.begin(), v.end(), 8);
    if (is_negative) {
        return -i;
    }
    return i;
}

std::vector<int8_t>
to_bytes(const boost::multiprecision::cpp_int& i)
{
    std::vector<int8_t> v;
    export_bits(i, std::back_inserter(v), 8);
    if (i < 0) {
        twos_complement(v);
        if (v[0] > 0) {
            // add -1 as the most significant to have a negative sign bit
            v.insert(v.begin(), -1);
        }
    } else {
        // add 0 as the most significant byte to have a positive sign bit
        if (v[0] < 0) {
            v.insert(v.begin(), 0);
        }
    }
    return v;
}
} // namespace pimpl

namespace serialization {
int32_t
hz_serializer<address>::get_factory_id()
{
    return F_ID;
}

int32_t
hz_serializer<address>::get_class_id()
{
    return ADDRESS;
}

void
hz_serializer<address>::write_data(const address& object,
                                   object_data_output& out)
{
    out.write<int32_t>(object.port_);
    out.write<byte>(object.type_);
    out.write(object.host_);
}

address
hz_serializer<address>::read_data(object_data_input& in)
{
    address object;
    object.port_ = in.read<int32_t>();
    object.type_ = in.read<byte>();
    object.host_ = in.read<std::string>();
    return object;
}
} // namespace serialization

iexecutor_service::iexecutor_service(const std::string& name,
                                     spi::ClientContext* context)
  : ProxyImpl(SERVICE_NAME, name, context)
  , consecutive_submits_(0)
  , last_submit_time_(0)
{}

std::vector<member>
iexecutor_service::select_members(const member_selector& member_selector)
{
    std::vector<member> selected;
    std::vector<member> members =
      get_context().get_client_cluster_service().get_member_list();
    for (const member& member : members) {
        if (member_selector.select(member)) {
            selected.push_back(member);
        }
    }
    if (selected.empty()) {
        BOOST_THROW_EXCEPTION(exception::rejected_execution(
          "IExecutorService::selectMembers",
          "No member could be selected with member selector"));
    }
    return selected;
}

std::pair<boost::future<protocol::ClientMessage>,
          std::shared_ptr<spi::impl::ClientInvocation>>
iexecutor_service::invoke_on_target(protocol::ClientMessage&& request,
                                    boost::uuids::uuid target)
{
    try {
        std::shared_ptr<spi::impl::ClientInvocation> clientInvocation =
          spi::impl::ClientInvocation::create(
            get_context(), request, get_name(), target);
        return std::make_pair(clientInvocation->invoke(), clientInvocation);
    } catch (exception::iexception&) {
        util::exception_util::rethrow(std::current_exception());
    }
    return std::pair<boost::future<protocol::ClientMessage>,
                     std::shared_ptr<spi::impl::ClientInvocation>>();
}

std::pair<boost::future<protocol::ClientMessage>,
          std::shared_ptr<spi::impl::ClientInvocation>>
iexecutor_service::invoke_on_partition_owner(protocol::ClientMessage&& request,
                                             int partition_id)
{
    try {
        std::shared_ptr<spi::impl::ClientInvocation> clientInvocation =
          spi::impl::ClientInvocation::create(
            get_context(), request, get_name(), partition_id);
        return std::make_pair(clientInvocation->invoke(), clientInvocation);
    } catch (exception::iexception&) {
        util::exception_util::rethrow(std::current_exception());
    }
    return std::pair<boost::future<protocol::ClientMessage>,
                     std::shared_ptr<spi::impl::ClientInvocation>>();
}

bool
iexecutor_service::is_sync_computation(bool prevent_sync)
{
    int64_t now = util::current_time_millis();

    int64_t last = last_submit_time_;
    last_submit_time_ = now;

    if (last + MIN_TIME_RESOLUTION_OF_CONSECUTIVE_SUBMITS < now) {
        consecutive_submits_ = 0;
        return false;
    }

    return !prevent_sync &&
           (consecutive_submits_++ % MAX_CONSECUTIVE_SUBMITS == 0);
}

address
iexecutor_service::get_member_address(const member& member)
{
    auto m =
      get_context().get_client_cluster_service().get_member(member.get_uuid());
    if (!m) {
        throw(exception::exception_builder<exception::hazelcast_>(
                "IExecutorService::getMemberAddress(Member)")
              << member << " is not available!")
          .build();
    }
    return m->get_address();
}

int
iexecutor_service::random_partition_id()
{
    auto& partitionService = get_context().get_partition_service();
    return rand() % partitionService.get_partition_count();
}

void
iexecutor_service::shutdown()
{
    auto request = protocol::codec::executorservice_shutdown_encode(get_name());
    invoke(request);
}

boost::future<bool>
iexecutor_service::is_shutdown()
{
    auto request =
      protocol::codec::executorservice_isshutdown_encode(get_name());
    return invoke_and_get_future<bool>(request);
}

boost::future<bool>
iexecutor_service::is_terminated()
{
    return is_shutdown();
}

const std::string client_properties::PROP_HEARTBEAT_TIMEOUT =
  "hazelcast_client_heartbeat_timeout";
const std::string client_properties::PROP_HEARTBEAT_TIMEOUT_DEFAULT = "60000";
const std::string client_properties::PROP_HEARTBEAT_INTERVAL =
  "hazelcast_client_heartbeat_interval";
const std::string client_properties::PROP_HEARTBEAT_INTERVAL_DEFAULT = "5000";
const std::string client_properties::PROP_REQUEST_RETRY_COUNT =
  "hazelcast_client_request_retry_count";
const std::string client_properties::PROP_REQUEST_RETRY_COUNT_DEFAULT = "20";
const std::string client_properties::PROP_REQUEST_RETRY_WAIT_TIME =
  "hazelcast_client_request_retry_wait_time";
const std::string client_properties::PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT = "1";

const std::string client_properties::PROP_AWS_MEMBER_PORT = "hz-port";
const std::string client_properties::PROP_AWS_MEMBER_PORT_DEFAULT = "5701";

const std::string client_properties::INVOCATION_RETRY_PAUSE_MILLIS =
  "hazelcast.client.invocation.retry.pause.millis";
const std::string client_properties::INVOCATION_RETRY_PAUSE_MILLIS_DEFAULT =
  "1000";

const std::string client_properties::INVOCATION_TIMEOUT_SECONDS =
  "hazelcast.client.invocation.timeout.seconds";
const std::string client_properties::INVOCATION_TIMEOUT_SECONDS_DEFAULT = "120";

const std::string client_properties::EVENT_THREAD_COUNT =
  "hazelcast.client.event.thread.count";
const std::string client_properties::EVENT_THREAD_COUNT_DEFAULT = "5";

const std::string client_properties::INTERNAL_EXECUTOR_POOL_SIZE =
  "hazelcast.client.internal.executor.pool.size";
const std::string client_properties::INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT = "3";

const std::string client_properties::SHUFFLE_MEMBER_LIST =
  "hazelcast.client.shuffle.member.list";
const std::string client_properties::SHUFFLE_MEMBER_LIST_DEFAULT = "true";

const std::string client_properties::MAX_CONCURRENT_INVOCATIONS =
  "hazelcast.client.max.concurrent.invocations";
const std::string client_properties::MAX_CONCURRENT_INVOCATIONS_DEFAULT =
  util::IOUtil::to_string<int32_t>(INT32_MAX);

const std::string client_properties::BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS =
  "hazelcast.client.invocation.backoff.timeout.millis";
const std::string
  client_properties::BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS_DEFAULT = "-1";

const std::string client_properties::STATISTICS_ENABLED =
  "hazelcast.client.statistics.enabled";
const std::string client_properties::STATISTICS_ENABLED_DEFAULT = "false";

const std::string client_properties::STATISTICS_PERIOD_SECONDS =
  "hazelcast.client.statistics.period.seconds";
const std::string client_properties::STATISTICS_PERIOD_SECONDS_DEFAULT = "3";

client_property::client_property(const std::string& name,
                                 const std::string& default_value)
  : name_(name)
  , default_value_(default_value)
{}

const std::string&
client_property::get_name() const
{
    return name_;
}

const std::string&
client_property::get_default_value() const
{
    return default_value_;
}

const char*
client_property::get_system_property() const
{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(                                                               \
  disable : 4996) // for 'getenv': This function or variable may be unsafe.
#endif
    return std::getenv(name_.c_str());
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
}

client_properties::client_properties(
  const std::unordered_map<std::string, std::string>& properties)
  : heartbeat_timeout_(PROP_HEARTBEAT_TIMEOUT, PROP_HEARTBEAT_TIMEOUT_DEFAULT)
  , heartbeat_interval_(PROP_HEARTBEAT_INTERVAL,
                        PROP_HEARTBEAT_INTERVAL_DEFAULT)
  , retry_count_(PROP_REQUEST_RETRY_COUNT, PROP_REQUEST_RETRY_COUNT_DEFAULT)
  , retry_wait_time_(PROP_REQUEST_RETRY_WAIT_TIME,
                     PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT)
  , aws_member_port_(PROP_AWS_MEMBER_PORT, PROP_AWS_MEMBER_PORT_DEFAULT)
  , invocation_retry_pause_millis_(INVOCATION_RETRY_PAUSE_MILLIS,
                                   INVOCATION_RETRY_PAUSE_MILLIS_DEFAULT)
  , invocation_timeout_seconds_(INVOCATION_TIMEOUT_SECONDS,
                                INVOCATION_TIMEOUT_SECONDS_DEFAULT)
  , event_thread_count_(EVENT_THREAD_COUNT, EVENT_THREAD_COUNT_DEFAULT)
  , internal_executor_pool_size_(INTERNAL_EXECUTOR_POOL_SIZE,
                                 INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT)
  , shuffle_member_list_(SHUFFLE_MEMBER_LIST, SHUFFLE_MEMBER_LIST_DEFAULT)
  , max_concurrent_invocations_(MAX_CONCURRENT_INVOCATIONS,
                                MAX_CONCURRENT_INVOCATIONS_DEFAULT)
  , backpressure_backoff_timeout_millis_(
      BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS,
      BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS_DEFAULT)
  , statistics_enabled_(STATISTICS_ENABLED, STATISTICS_ENABLED_DEFAULT)
  , statistics_period_seconds_(STATISTICS_PERIOD_SECONDS,
                               STATISTICS_PERIOD_SECONDS_DEFAULT)
  , backup_timeout_millis_(OPERATION_BACKUP_TIMEOUT_MILLIS,
                           OPERATION_BACKUP_TIMEOUT_MILLIS_DEFAULT)
  , fail_on_indeterminate_state_(FAIL_ON_INDETERMINATE_OPERATION_STATE,
                                 FAIL_ON_INDETERMINATE_OPERATION_STATE_DEFAULT)
  , cloud_base_url_(CLOUD_URL_BASE, CLOUD_URL_BASE_DEFAULT)
  , partition_arg_cache_size_(PARTITION_ARGUMENT_CACHE_SIZE,
                              PARTITION_ARGUMENT_CACHE_SIZE_DEFAULT)
  , properties_map_(properties)
{}

const client_property&
client_properties::get_heartbeat_timeout() const
{
    return heartbeat_timeout_;
}

const client_property&
client_properties::get_heartbeat_interval() const
{
    return heartbeat_interval_;
}

const client_property&
client_properties::get_aws_member_port() const
{
    return aws_member_port_;
}

const client_property&
client_properties::get_invocation_retry_pause_millis() const
{
    return invocation_retry_pause_millis_;
}

const client_property&
client_properties::get_invocation_timeout_seconds() const
{
    return invocation_timeout_seconds_;
}

const client_property&
client_properties::get_event_thread_count() const
{
    return event_thread_count_;
}

const client_property&
client_properties::get_internal_executor_pool_size() const
{
    return internal_executor_pool_size_;
}

const client_property&
client_properties::get_shuffle_member_list() const
{
    return shuffle_member_list_;
}

const client_property&
client_properties::get_max_concurrent_invocations() const
{
    return max_concurrent_invocations_;
}

const client_property&
client_properties::get_backpressure_backoff_timeout_millis() const
{
    return backpressure_backoff_timeout_millis_;
}

const client_property&
client_properties::get_statistics_enabled() const
{
    return statistics_enabled_;
}

const client_property&
client_properties::get_statistics_period_seconds() const
{
    return statistics_period_seconds_;
}

std::string
client_properties::get_string(const client_property& property) const
{
    std::unordered_map<std::string, std::string>::const_iterator valueIt =
      properties_map_.find(property.get_name());
    if (valueIt != properties_map_.end()) {
        return valueIt->second;
    }

    const char* value = property.get_system_property();
    if (value != NULL) {
        return value;
    }

    return property.get_default_value();
}

bool
client_properties::get_boolean(const client_property& property) const
{
    return util::IOUtil::to_value<bool>(get_string(property));
}

int32_t
client_properties::get_integer(const client_property& property) const
{
    return util::IOUtil::to_value<int32_t>(get_string(property));
}

int64_t
client_properties::get_long(const client_property& property) const
{
    return util::IOUtil::to_value<int64_t>(get_string(property));
}

const client_property&
client_properties::backup_timeout_millis() const
{
    return backup_timeout_millis_;
}

const client_property&
client_properties::fail_on_indeterminate_state() const
{
    return fail_on_indeterminate_state_;
}

const client_property&
client_properties::cloud_base_url() const
{
    return cloud_base_url_;
}

const client_property&
client_properties::partition_arg_cache_size() const
{
    return partition_arg_cache_size_;
}

namespace exception {
iexception::iexception(std::string exception_name,
                       std::string source,
                       std::string message,
                       std::string details,
                       int32_t error_no,
                       std::exception_ptr cause,
                       bool is_runtime,
                       bool retryable)
  : src_(std::move(source))
  , msg_(std::move(message))
  , details_(std::move(details))
  , error_code_(error_no)
  , cause_(std::move(cause))
  , runtime_exception_(is_runtime)
  , retryable_(retryable)
  , report_((boost::format("%1% {%2%. Error code:%3%, Details:%4%.} at %5%.") %
             exception_name % msg_ % error_code_ % details_ % src_)
              .str())
{}

iexception::~iexception() noexcept = default;

char const*
iexception::what() const noexcept
{
    return report_.c_str();
}

const std::string&
iexception::get_source() const
{
    return src_;
}

const std::string&
iexception::get_message() const
{
    return msg_;
}

std::ostream&
operator<<(std::ostream& os, const iexception& exception)
{
    os << exception.what();
    return os;
}

const std::string&
iexception::get_details() const
{
    return details_;
}

int32_t
iexception::get_error_code() const
{
    return error_code_;
}

bool
iexception::is_runtime() const
{
    return runtime_exception_;
}

bool
iexception::is_retryable() const
{
    return retryable_;
}

iexception::iexception() = default;

retryable_hazelcast::retryable_hazelcast(std::string source,
                                         std::string message,
                                         std::string details,
                                         std::exception_ptr cause)
  : retryable_hazelcast("retryable_hazelcast",
                        protocol::RETRYABLE_HAZELCAST,
                        std::move(source),
                        std::move(message),
                        std::move(details),
                        std::move(cause),
                        true,
                        true)
{}

retryable_hazelcast::retryable_hazelcast(std::string error_name,
                                         int32_t error_code,
                                         std::string source,
                                         std::string message,
                                         std::string details,
                                         std::exception_ptr cause,
                                         bool runtime,
                                         bool retryable)
  : hazelcast_(std::move(error_name),
               error_code,
               std::move(source),
               std::move(message),
               std::move(details),
               std::move(cause),
               runtime,
               retryable)
{}

member_left::member_left(std::string source,
                         std::string message,
                         std::string details,
                         std::exception_ptr cause)
  : execution("member_left",
              protocol::MEMBER_LEFT,
              std::move(source),
              std::move(message),
              std::move(details),
              std::move(cause),
              false,
              true)
{}

consistency_lost::consistency_lost(std::string source,
                                   std::string message,
                                   std::string details,
                                   std::exception_ptr cause)
  : hazelcast_("consistency_lost",
               protocol::CONSISTENCY_LOST_EXCEPTION,
               std::move(source),
               std::move(message),
               std::move(details),
               std::move(cause),
               true,
               false)
{}

query::query(std::string source,
             std::string message,
             std::string details,
             std::exception_ptr cause)
  : hazelcast_(std::move(source),
               std::move(message),
               std::move(details),
               std::move(cause),
               false)
{
}

query::query(int32_t code,
             std::string message,
             std::exception_ptr cause,
             boost::uuids::uuid originating_member_id,
             std::string suggestion)
  : hazelcast_("", std::move(message), "", std::move(cause))
  , code_(code)
  , suggestion_(std::move(suggestion))
  , originating_member_uuid_(originating_member_id)
{
}

int32_t
query::code() const
{
    return code_;
}

const std::string&
query::suggestion() const
{
    return suggestion_;
}

const boost::uuids::uuid&
query::originating_member_uuid() const
{
    return originating_member_uuid_;
}

invocation_might_contain_compact_data::invocation_might_contain_compact_data(
  std::string source,
  const spi::impl::ClientInvocation& invocation)
  : hazelcast_{
      std::move(source),
      boost::str(
        boost::format(
          "The invocation %1% might contain Compact serialized "
          "data and it is not safe to invoke it when the client is not "
          "yet initialized on the cluster") %
        invocation)
  }
{
}

} // namespace exception
} // namespace client

boost::future<client::hazelcast_client>
new_client()
{
    return boost::async([]() { return client::hazelcast_client(); });
}

boost::future<client::hazelcast_client>
new_client(client::client_config config)
{
    return boost::async(
      [](client::client_config&& c) {
          return client::hazelcast_client(std::move(c));
      },
      std::move(config));
}
} // namespace hazelcast

namespace std {
std::size_t
hash<hazelcast::client::address>::operator()(
  const hazelcast::client::address& address) const noexcept
{
    std::size_t seed = 0;
    boost::hash_combine(seed, address.get_host());
    boost::hash_combine(seed, address.get_port());
    boost::hash_combine(seed, address.type_);
    return seed;
}

std::size_t
hash<hazelcast::client::big_decimal>::operator()(
  const hazelcast::client::big_decimal& dec) const
{
    std::size_t seed = 0;
    boost::hash_combine(seed, dec.unscaled);
    boost::hash_combine(seed, dec.scale);
    return seed;
}

std::size_t
hash<hazelcast::client::local_time>::operator()(
  const hazelcast::client::local_time& v) const
{
    std::size_t seed = 0;
    boost::hash_combine(seed, v.hours);
    boost::hash_combine(seed, v.minutes);
    boost::hash_combine(seed, v.seconds);
    boost::hash_combine(seed, v.nanos);
    return seed;
}

std::size_t
hash<hazelcast::client::local_date>::operator()(
  const hazelcast::client::local_date& v) const
{
    std::size_t seed = 0;
    boost::hash_combine(seed, v.year);
    boost::hash_combine(seed, v.month);
    boost::hash_combine(seed, v.day_of_month);
    return seed;
}

std::size_t
hash<hazelcast::client::local_date_time>::operator()(
  const hazelcast::client::local_date_time& v) const
{
    std::size_t seed = 0;
    boost::hash_combine<hazelcast::client::local_date>(seed, v.date);
    boost::hash_combine<hazelcast::client::local_time>(seed, v.time);
    return seed;
}

std::size_t
hash<hazelcast::client::offset_date_time>::operator()(
  const hazelcast::client::offset_date_time& v) const
{
    std::size_t seed = 0;
    boost::hash_combine<hazelcast::client::local_date_time>(seed, v.date_time);
    boost::hash_combine(seed, v.zone_offset_in_seconds);
    return seed;
}
} // namespace std
