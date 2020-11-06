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

#include "hazelcast/util/hazelcast_dll.h"
#include "hazelcast/client/spi/DefaultObjectNamespace.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientContext;
            class ClientProxy;

            class HAZELCAST_API ProxyManager {
            public:
                typedef std::unordered_map<DefaultObjectNamespace, std::shared_future<std::shared_ptr<ClientProxy>>> proxy_map;

                explicit ProxyManager(ClientContext &context);

                template<typename T>
                std::shared_ptr<T> get_or_create_proxy(const std::string &service, const std::string &id) {
                    DefaultObjectNamespace ns(service, id);

                    std::shared_future<std::shared_ptr<ClientProxy>> proxyFuture;
                    std::promise<std::shared_ptr<ClientProxy>> promise;
                    bool insertedEntry = false;
                    {
                        std::lock_guard<std::mutex> guard(lock_);
                        auto it = proxies_.find(ns);
                        if (it != proxies_.end()) {
                            proxyFuture = it->second;
                        } else {
                            proxies_.insert({ns, promise.get_future().share()});
                            insertedEntry = true;
                        }
                    }

                    if (proxyFuture.valid()) {
                        return std::static_pointer_cast<T>(proxyFuture.get());
                    }

                    try {
                        auto clientProxy = std::shared_ptr<T>(new T(id, &client_));
                        initialize(std::static_pointer_cast<ClientProxy>(clientProxy));
                        promise.set_value(std::static_pointer_cast<ClientProxy>(clientProxy));
                        return clientProxy;
                    } catch (exception::IException &) {
                        promise.set_exception(std::current_exception());
                        std::lock_guard<std::mutex> guard(lock_);
                        if (insertedEntry) {
                            proxies_.erase(ns);
                        }
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
                boost::future<void> destroy_proxy(ClientProxy &proxy);

                void init();

                void destroy();

            private:
                void initialize(const std::shared_ptr<ClientProxy> &client_proxy);

                proxy_map proxies_;
                std::mutex lock_;
                ClientContext &client_;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
