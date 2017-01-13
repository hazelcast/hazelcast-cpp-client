/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
// Created by ihsan demir on 25 Nov 2016.

#ifndef HAZELCAST_CLIENT_SPI_PROXYMANAGER_H_
#define HAZELCAST_CLIENT_SPI_PROXYMANAGER_H_

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/spi/ObjectNamespace.h"
#include "hazelcast/client/spi/DefaultObjectNamespace.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            class HAZELCAST_API ProxyManager {
            public:
                ProxyManager(ClientContext &context) : clientContext(context) {
                    (void) clientContext;
                }

                template <typename FACTORY>
                boost::shared_ptr<ClientProxy> getOrCreateProxy(
                        const std::string &service, const std::string &id, FACTORY &factory) {
                    DefaultObjectNamespace ns(service, id);
                    boost::shared_ptr<ClientProxy> proxy = proxies.get(ns);
                    if (proxy.get() != NULL) {
                        return proxy;
                    }
                    boost::shared_ptr<ClientProxy> clientProxy = factory.create(id);
                    boost::shared_ptr<ClientProxy> current = proxies.putIfAbsent(ns, clientProxy);
                    if (current.get() != NULL) {
                        return current;
                    }
                    try {
                        initializeWithRetry(clientProxy);
                    } catch (exception::IException &e) {
                        proxies.remove(ns);
                        throw e;
                    }
                    return clientProxy;
                }
            private:
                void initializeWithRetry(boost::shared_ptr<ClientProxy> clientProxy) {
                    // TODO: Implement like java
                    clientProxy->onInitialize();
                }

                ClientContext &clientContext;
                //TODO: Change ClientProxy to ClientProxyFuture as in java
                util::SynchronizedMap<DefaultObjectNamespace, ClientProxy> proxies;
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_PROXYMANAGER_H_

