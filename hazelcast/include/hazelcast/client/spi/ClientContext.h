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
#pragma once

#include <string>
#include <memory>
#include <boost/uuid/uuid.hpp>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    class logger;

    namespace cp {
        namespace internal {
            namespace session {
                class proxy_session_manager;
            }
        }
    }
    namespace client {
        class Cluster;
        class HazelcastClient;
        class ClientConfig;
        class ClientProperties;

        namespace serialization {
            namespace pimpl {
                class SerializationService;
            }
        }

        namespace impl {
            class HazelcastClientInstanceImpl;
            class ClientLockReferenceIdGenerator;

            namespace statistics {
                class Statistics;
            }
        }

        namespace connection {
            class ClientConnectionManagerImpl;
            class Connection;
        }

        namespace internal {
            namespace nearcache {
                class NearCacheManager;
            }
        }

        namespace protocol {
            class ClientExceptionFactory;
        }

        namespace spi {
            class ClientListenerService;

            class LifecycleService;

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
                }
            }

            class HAZELCAST_API ClientContext {
            public:
                // This constructor is used from tests
                explicit ClientContext(const client::HazelcastClient &hazelcastClient);

                explicit ClientContext(client::impl::HazelcastClientInstanceImpl &hazelcastClient);

                serialization::pimpl::SerializationService &getSerializationService();

                impl::ClientClusterServiceImpl & getClientClusterService();

                impl::ClientInvocationServiceImpl &getInvocationService();

                ClientConfig &getClientConfig();

                impl::ClientPartitionServiceImpl & getPartitionService();

                LifecycleService &getLifecycleService();

                spi::impl::listener::listener_service_impl &getClientListenerService();

                connection::ClientConnectionManagerImpl &getConnectionManager();

                internal::nearcache::NearCacheManager &getNearCacheManager();

                ClientProperties &getClientProperties();

                Cluster &getCluster();

                std::shared_ptr<impl::sequence::CallIdSequence> &getCallIdSequence() const;

                const protocol::ClientExceptionFactory &getClientExceptionFactory() const;

                const std::string &getName() const;

                impl::ClientExecutionServiceImpl &getClientExecutionService() const;

                const std::shared_ptr<client::impl::ClientLockReferenceIdGenerator> &getLockReferenceIdGenerator();

                std::shared_ptr<client::impl::HazelcastClientInstanceImpl> getHazelcastClientImplementation();

                spi::ProxyManager &getProxyManager();

                logger &getLogger();

                client::impl::statistics::Statistics &getClientstatistics();

                spi::impl::listener::cluster_view_listener &get_cluster_view_listener();

                boost::uuids::uuid random_uuid();

                cp::internal::session::proxy_session_manager &get_proxy_session_manager();
            private:
                client::impl::HazelcastClientInstanceImpl &hazelcastClient;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


