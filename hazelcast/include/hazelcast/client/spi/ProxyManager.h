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

#include "hazelcast/util/export.h"
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
                typedef std::unordered_map<DefaultObjectNamespace, boost::shared_future<std::shared_ptr<ClientProxy>>> proxy_map;

                explicit ProxyManager(ClientContext &context);

                template<typename T>
                boost::shared_future<std::shared_ptr<T>> get_or_create_proxy(const std::string &service, const std::string &id) {
                    DefaultObjectNamespace ns(service, id);

                    std::lock_guard<std::mutex> guard(lock_);
                    auto it = proxies_.find(ns);
                    if (it != proxies_.end()) {
                        auto proxy_future = it->second;
                        return convert_to_concrete_proxy_future<T>(proxy_future);
                    }

                    auto concrete_proxy = std::shared_ptr<T>(new T(id, &client_));
                    auto client_proxy = std::static_pointer_cast<ClientProxy>(concrete_proxy);
                    auto proxy_future = initialize(client_proxy).then(boost::launch::sync, [=](boost::future<void> f) {
                        try {
                            f.get();
                            return client_proxy;
                        } catch (...) {
                            std::lock_guard<std::mutex> guard(lock_);
                            proxies_.erase(ns);
                            throw;
                        }
                    }).share();

                    proxies_.insert({ns, proxy_future});
                    return convert_to_concrete_proxy_future<T>(proxy_future);
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
                boost::future<void> initialize(const std::shared_ptr<ClientProxy> &client_proxy);

                template <typename T>
                boost::shared_future<std::shared_ptr<T>>
                convert_to_concrete_proxy_future(boost::shared_future<std::shared_ptr<ClientProxy>> proxy_future) {
                    return proxy_future.then(boost::launch::sync,
                                             [](boost::shared_future<std::shared_ptr<ClientProxy>> f) {
                                                 return std::static_pointer_cast<T>(f.get());
                                             });
                }

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
