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
//
// Created by sancar koyunlu on 5/23/13.

#ifndef HAZELCAST_CLIENT_CONTEXT
#define HAZELCAST_CLIENT_CONTEXT

#include <string>
#include <memory>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class ILogger;
    }
    namespace client {

        class Cluster;

        namespace serialization {
            namespace pimpl {
                class SerializationService;
            }
        }
        class HazelcastClient;

        class ClientConfig;

        class ClientProperties;

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
            class ClientInvocationService;

            class ClientPartitionService;

            class ClientListenerService;

            class LifecycleService;

            class ClientInvocationService;

            class ClientClusterService;

            class ProxyManager;

            namespace impl {
                class ClientExecutionServiceImpl;

                namespace sequence {
                    class CallIdSequence;
                }
            }

            class HAZELCAST_API ClientContext {
            public:
                // This constructor is used from tests
                ClientContext(const client::HazelcastClient &hazelcastClient);

                ClientContext(client::impl::HazelcastClientInstanceImpl &hazelcastClient);

                serialization::pimpl::SerializationService &getSerializationService();

                ClientClusterService &getClientClusterService();

                ClientInvocationService &getInvocationService();

                ClientConfig &getClientConfig();

                ClientPartitionService &getPartitionService();

                LifecycleService &getLifecycleService();

                ClientListenerService &getClientListenerService();

                connection::ClientConnectionManagerImpl &getConnectionManager();

                internal::nearcache::NearCacheManager &getNearCacheManager();

                ClientProperties &getClientProperties();

                Cluster &getCluster();

                std::shared_ptr<impl::sequence::CallIdSequence> &getCallIdSequence() const;

                const protocol::ClientExceptionFactory &getClientExceptionFactory() const;

                const std::string &getName() const;

                impl::ClientExecutionServiceImpl &getClientExecutionService() const;

                void onClusterConnect(const std::shared_ptr<connection::Connection> &ownerConnection);

                const std::shared_ptr<client::impl::ClientLockReferenceIdGenerator> &getLockReferenceIdGenerator();

                std::shared_ptr<client::impl::HazelcastClientInstanceImpl> getHazelcastClientImplementation();

                spi::ProxyManager &getProxyManager();

                util::ILogger &getLogger();

                client::impl::statistics::Statistics &getClientstatistics();
            private:
                client::impl::HazelcastClientInstanceImpl &hazelcastClient;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif // HAZELCAST_CLIENT_CONTEXT
