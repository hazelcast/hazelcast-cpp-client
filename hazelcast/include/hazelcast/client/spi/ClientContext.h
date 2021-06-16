/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include <string>
#include <memory>
#include <boost/uuid/uuid.hpp>

#include "hazelcast/util/export.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
class logger;

namespace cp {
namespace internal {
namespace session {
class proxy_session_manager;
}
} // namespace internal
} // namespace cp
namespace client {
class cluster;
class hazelcast_client;
class client_config;
class client_properties;

namespace serialization {
namespace pimpl {
class SerializationService;
}
} // namespace serialization

namespace impl {
class hazelcast_client_instance_impl;
class ClientLockReferenceIdGenerator;

namespace statistics {
class Statistics;
}
} // namespace impl

namespace connection {
class ClientConnectionManagerImpl;
class Connection;
} // namespace connection

namespace internal {
namespace nearcache {
class NearCacheManager;
}
} // namespace internal

namespace protocol {
class ClientExceptionFactory;
}

namespace spi {
class ClientListenerService;

class lifecycle_service;

class ProxyManager;

namespace impl {
class ClientExecutionServiceImpl;
class ClientInvocationServiceImpl;
class ClientPartitionServiceImpl;
class ClientClusterServiceImpl;

namespace sequence {
class CallIdSequence;
}

namespace listener {
class cluster_view_listener;
class listener_service_impl;
} // namespace listener
} // namespace impl

class HAZELCAST_API ClientContext
{
public:
    // This constructor is used from tests
    explicit ClientContext(const client::hazelcast_client& hazelcast_client);

    explicit ClientContext(client::impl::hazelcast_client_instance_impl& hazelcast_client);

    serialization::pimpl::SerializationService& get_serialization_service();

    impl::ClientClusterServiceImpl& get_client_cluster_service();

    impl::ClientInvocationServiceImpl& get_invocation_service();

    client_config& get_client_config();

    impl::ClientPartitionServiceImpl& get_partition_service();

    lifecycle_service& get_lifecycle_service();

    spi::impl::listener::listener_service_impl& get_client_listener_service();

    connection::ClientConnectionManagerImpl& get_connection_manager();

    internal::nearcache::NearCacheManager& get_near_cache_manager();

    client_properties& get_client_properties();

    cluster& get_cluster();

    std::shared_ptr<impl::sequence::CallIdSequence>& get_call_id_sequence() const;

    const protocol::ClientExceptionFactory& get_client_exception_factory() const;

    const std::string& get_name() const;

    impl::ClientExecutionServiceImpl& get_client_execution_service() const;

    const std::shared_ptr<client::impl::ClientLockReferenceIdGenerator>&
    get_lock_reference_id_generator();

    std::shared_ptr<client::impl::hazelcast_client_instance_impl>
    get_hazelcast_client_implementation();

    spi::ProxyManager& get_proxy_manager();

    logger& get_logger();

    client::impl::statistics::Statistics& get_clientstatistics();

    spi::impl::listener::cluster_view_listener& get_cluster_view_listener();

    boost::uuids::uuid random_uuid();

    cp::internal::session::proxy_session_manager& get_proxy_session_manager();

private:
    client::impl::hazelcast_client_instance_impl& hazelcast_client_;
};
} // namespace spi
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
