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
#include <future>
#include <unordered_map>

#include <boost/any.hpp>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/spi/DefaultObjectNamespace.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class Address;

        namespace spi {
            class ClientContext;
            class ClientProxy;
            class ClientProxyFactory;

            class HAZELCAST_API ProxyManager {
            public:
                typedef std::unordered_map<DefaultObjectNamespace, std::shared_future<std::shared_ptr<ClientProxy>>> proxy_map;

                ProxyManager(ClientContext &context);

                template<typename T>
                std::shared_ptr<T> getOrCreateProxy(const std::string &service, const std::string &id) {
                    DefaultObjectNamespace ns(service, id);

                    std::shared_future<std::shared_ptr<ClientProxy>> proxyFuture;
                    std::promise<std::shared_ptr<ClientProxy>> promise;
                    std::pair<proxy_map::iterator, bool> insertedEntry;
                    {
                        std::lock_guard<std::mutex> guard(lock);
                        auto it = proxies.find(ns);
                        if (it != proxies.end()) {
                            proxyFuture = it->second;
                        } else {
                            insertedEntry = proxies.insert({ns, promise.get_future().share()});
                            assert(insertedEntry.second);
                        }
                    }

                    if (proxyFuture.valid()) {
                        return std::static_pointer_cast<T>(proxyFuture.get());
                    }

                    try {
                        auto clientProxy = std::shared_ptr<T>(new T(id, &client));
                        initializeWithRetry(clientProxy);
                        promise.set_value(clientProxy);
                        return clientProxy;
                    } catch (exception::IException &e) {
                        promise.set_exception(std::current_exception());
                        std::lock_guard<std::mutex> guard(lock);
                        proxies.erase(insertedEntry.first);
                        throw;
                    }
                }

                /**
                 * Destroys the given proxy in a cluster-wide way.
                 * <p>
                 * Upon successful completion the proxy is unregistered in this proxy
                 * manager, all local resources associated with the proxy are released and
                 * a distributed object destruction operation is issued to the cluster.
                 * <p>
                 * If the given proxy instance is not registered in this proxy manager, the
                 * proxy instance is considered stale. In this case, this stale instance is
                 * a subject to a local-only destruction and its registered counterpart, if
                 * there is any, is a subject to a cluster-wide destruction.
                 *
                 * @param proxy the proxy to destroy.
                 */
                boost::future<void> destroyProxy(ClientProxy &proxy);

                void init();

                void destroy();

            private:
                void initializeWithRetry(const std::shared_ptr<ClientProxy> &clientProxy);

                std::shared_ptr<Address> findNextAddressToSendCreateRequest();

                void initialize(const std::shared_ptr<ClientProxy> &clientProxy);

                proxy_map proxies;
                std::mutex lock;
                std::chrono::steady_clock::duration invocationTimeout;
                std::chrono::steady_clock::duration invocationRetryPause;
                ClientContext &client;

                bool isRetryable(exception::IException &exception);

                void sleepForProxyInitRetry();
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
