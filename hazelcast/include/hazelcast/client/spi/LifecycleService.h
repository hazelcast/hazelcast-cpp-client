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

#include <mutex>
#include <boost/thread/latch.hpp>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/util/UuidUtil.h"

#include <unordered_map>

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

                LifecycleService(ClientContext &clientContext, const std::vector<LifecycleListener> &lifecycleListeners,
                                 LoadBalancer *const loadBalancer, Cluster &cluster);

                virtual ~LifecycleService();

                bool start();

                void fireLifecycleEvent(const LifecycleEvent &lifecycleEvent);

                void shutdown();

                template<typename T>
                std::string addLifecycleListener(T &&lifecycleListener) {
                    std::lock_guard<std::mutex> lg(listenerLock);

                    const std::string registrationId = util::UuidUtil::newUnsecureUuidString();

                    listeners.emplace(registrationId, std::forward<T>(lifecycleListener));
                    return registrationId;
                }

                bool removeLifecycleListener(const std::string &registrationId);

                bool isRunning();

            private:
                ClientContext &clientContext;
                std::unordered_map<std::string, LifecycleListener> listeners;
                std::mutex listenerLock;
                util::AtomicBoolean active;
                LoadBalancer *loadBalancer;
                Cluster &cluster;
                boost::latch shutdownCompletedLatch;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

