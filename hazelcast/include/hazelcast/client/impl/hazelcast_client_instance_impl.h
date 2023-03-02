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
#pragma once

#include <atomic>
#include <random>
#include <boost/uuid/uuid.hpp>

#include "hazelcast/client/transaction_context.h"
#include "hazelcast/client/cluster.h"
#include "hazelcast/client/client_config.h"
#include "hazelcast/client/client_properties.h"
#include "hazelcast/client/spi/lifecycle_service.h"
#include "hazelcast/client/spi/ProxyManager.h"
#include "hazelcast/client/protocol/ClientExceptionFactory.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientTransactionManagerServiceImpl.h"
#include "hazelcast/client/local_endpoint.h"
#include "hazelcast/cp/cp_impl.h"
#include "hazelcast/client/sql/sql_service.h"
#include "hazelcast/client/serialization/pimpl/compact/compact.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
class logger;
namespace client {
class hazelcast_client;
class imap;
namespace connection {
class ClientConnectionManagerImpl;
class AddressProvider;
} // namespace connection

namespace internal {
namespace nearcache {
class NearCacheManager;
} // namespace nearcache
} // namespace internal

namespace serialization {
namespace pimpl {
class SerializationService;
} // namespace pimpl
} // namespace serialization
namespace spi {
class ClientContext;
class lifecycle_service;

namespace impl {
class ClientExecutionServiceImpl;
class ClientInvocationServiceImpl;
class ClientPartitionServiceImpl;
class Statistics;
namespace listener {
class listener_service_impl;
} // namespace listener
namespace sequence {
class CallIdSequence;
} // namespace sequence
} // namespace impl
} // namespace spi

class transaction_options;

class cluster;

class cluster_view_listener;

namespace impl {
class ClientLockReferenceIdGenerator;

class HAZELCAST_API hazelcast_client_instance_impl
  : public std::enable_shared_from_this<hazelcast_client_instance_impl>
{
    friend class spi::ClientContext;

public:
    /**
     * Constructs a hazelcastClient with given ClientConfig.
     * Note: client_config will be copied.
     * @param config client configuration to start the client with
     */
    explicit hazelcast_client_instance_impl(hazelcast_client& client,
                                            client_config config);

    /**
     * Destructor
     */
    ~hazelcast_client_instance_impl();

    void start();

    /**
     * Returns the name of this Hazelcast instance.
     *
     * @return name of this Hazelcast instance
     */
    const std::string& get_name() const;

    /**
     *
     * @tparam T type of the distributed object
     * @param name name of the distributed object.
     * @returns distributed object
     */
    template<typename T>
    boost::shared_future<std::shared_ptr<T>> get_distributed_object(
      const std::string& name)
    {
        return proxy_manager_.get_or_create_proxy<T>(T::SERVICE_NAME, name);
    }

    /**
     *
     * @return configuration of this Hazelcast client.
     */
    client_config& get_client_config();

    /**
     * Creates a new TransactionContext associated with the current thread using
     * default options.
     *
     * @return new TransactionContext
     */
    transaction_context new_transaction_context();

    /**
     * Creates a new TransactionContext associated with the current thread with
     * given options.
     *
     * @param options options for this transaction
     * @return new TransactionContext
     */
    transaction_context new_transaction_context(
      const transaction_options& options);

    /**
     * Returns the Cluster that connected Hazelcast instance is a part of.
     * Cluster interface allows you to add listener for membership
     * events and learn more about the cluster.
     *
     * @return cluster
     */
    cluster& get_cluster();

    local_endpoint get_local_endpoint() const;

    /**
     * Add listener to listen lifecycle events.
     *
     * Warning 1: If listener should do a time consuming operation, off-load the
     * operation to another thread. otherwise it will slow down the system.
     *
     * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
     *
     * @param lifecycleListener Listener object
     */
    boost::uuids::uuid add_lifecycle_listener(
      lifecycle_listener&& lifecycle_listener);

    /**
     * Remove lifecycle listener
     * @param lifecycleListener
     * @return true if removed successfully
     */
    bool remove_lifecycle_listener(const boost::uuids::uuid& registration_id);

    /**
     * Shuts down this hazelcast_client.
     */
    void shutdown();

    spi::lifecycle_service& get_lifecycle_service();

    internal::nearcache::NearCacheManager& get_near_cache_manager();

    serialization::pimpl::SerializationService& get_serialization_service();

    const protocol::ClientExceptionFactory& get_exception_factory() const;

    void on_cluster_restart();

    const std::shared_ptr<ClientLockReferenceIdGenerator>&
    get_lock_reference_id_generator() const;

    spi::ProxyManager& get_proxy_manager();

    const std::shared_ptr<logger>& get_logger() const;

    boost::uuids::uuid random_uuid();

    cp::cp_subsystem& get_cp_subsystem();

    sql::sql_service& get_sql();

    void send_state_to_cluster();

    /**
     * Returns {@code true} if we need to check the urgent invocations, by
     * examining the local registry of the schema service.
     */
    bool should_check_urgent_invocations() const;

private:
    client_config client_config_;
    client_properties client_properties_;
    spi::ClientContext client_context_;
    serialization::pimpl::default_schema_service schema_service_;
    serialization::pimpl::SerializationService serialization_service_;
    std::shared_ptr<connection::ClientConnectionManagerImpl>
      connection_manager_;
    std::unique_ptr<internal::nearcache::NearCacheManager> near_cache_manager_;
    spi::impl::ClientClusterServiceImpl cluster_service_;
    std::shared_ptr<spi::impl::ClientPartitionServiceImpl> partition_service_;
    std::shared_ptr<spi::impl::ClientExecutionServiceImpl> execution_service_;
    std::unique_ptr<spi::impl::ClientInvocationServiceImpl> invocation_service_;
    std::shared_ptr<spi::impl::listener::listener_service_impl>
      listener_service_;
    spi::impl::ClientTransactionManagerServiceImpl transaction_manager_;
    cluster cluster_;
    spi::lifecycle_service lifecycle_service_;
    spi::ProxyManager proxy_manager_;
    std::shared_ptr<spi::impl::sequence::CallIdSequence> call_id_sequence_;
    std::unique_ptr<statistics::Statistics> statistics_;
    protocol::ClientExceptionFactory exception_factory_;
    std::string instance_name_;
    static std::atomic<int32_t> CLIENT_ID;
    int32_t id_;
    std::shared_ptr<ClientLockReferenceIdGenerator>
      lock_reference_id_generator_;
    std::shared_ptr<logger> logger_;
    std::shared_ptr<spi::impl::listener::cluster_view_listener>
      cluster_listener_;
    std::mt19937 random_generator_;
    boost::uuids::basic_random_generator<std::mt19937> uuid_generator_;
    std::mutex uuid_generator_lock_;
    cp::cp_subsystem cp_subsystem_;
    sql::sql_service sql_service_;
    cp::internal::session::proxy_session_manager proxy_session_manager_;

    hazelcast_client_instance_impl(const hazelcast_client_instance_impl& rhs) =
      delete;

    void operator=(const hazelcast_client_instance_impl& rhs) = delete;

    std::shared_ptr<spi::impl::listener::listener_service_impl>
    init_listener_service();

    std::shared_ptr<spi::impl::ClientExecutionServiceImpl>
    init_execution_service();

    std::unique_ptr<connection::AddressProvider> create_address_provider();

    void initalize_near_cache_manager();

    void check_discovery_configuration_consistency(bool address_list_provided,
                                                   bool aws_enabled,
                                                   bool cloud_enabled);
};

template<>
boost::shared_future<std::shared_ptr<imap>> HAZELCAST_API
hazelcast_client_instance_impl::get_distributed_object(const std::string& name);
} // namespace impl
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
