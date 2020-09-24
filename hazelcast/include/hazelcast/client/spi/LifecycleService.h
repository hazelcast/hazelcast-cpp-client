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

#include <random>
#include <unordered_map>
#include <mutex>
#include <unordered_set>
#include <atomic>
#include <boost/thread/latch.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/container_hash/hash.hpp>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/LifecycleListener.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class LifecycleEvent;

        class ClientConfig;

        class LoadBalancer;

        class Cluster;

        namespace spi {

            class ClientContext;

            class HAZELCAST_API LifecycleService {
            public:

                LifecycleService(ClientContext &clientContext, 
                                 const std::vector<LifecycleListener> &lifecycleListeners,
                                 LoadBalancer *const loadBalancer, Cluster &cluster);

                virtual ~LifecycleService();

                bool start();

                void fireLifecycleEvent(const LifecycleEvent &lifecycleEvent);

                void shutdown();

                boost::uuids::uuid addListener(LifecycleListener &&lifecycleListener);

                bool removeListener(const boost::uuids::uuid &registrationId);

                bool isRunning();

            private:
                ClientContext &client_context_;
                std::unordered_map<boost::uuids::uuid, LifecycleListener, boost::hash<boost::uuids::uuid>> listeners_;
                std::mutex listener_lock_;
                std::atomic<bool> active_{ false };
                LoadBalancer *load_balancer_;
                Cluster &cluster_;
                boost::latch shutdown_completed_latch_;
                std::mt19937 random_generator_{std::random_device{}()};
                boost::uuids::basic_random_generator<std::mt19937> uuid_generator_{random_generator_};

                void wait_for_initial_membership_event() const;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

