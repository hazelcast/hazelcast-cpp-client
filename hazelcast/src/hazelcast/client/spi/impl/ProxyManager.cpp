/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/spi/ProxyManager.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/ClientProxy.h"
#include "hazelcast/client/spi/ClientProxyFactory.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            ProxyManager::ProxyManager(ClientContext &context) {
            }

            boost::shared_ptr<ClientProxy> ProxyManager::getOrCreateProxy(
                    const std::string &service, const std::string &id, ClientProxyFactory &factory) {
                DefaultObjectNamespace ns(service, id);
                boost::shared_ptr<util::Future<boost::shared_ptr<ClientProxy> > > proxyFuture = proxies.get(ns);
                if (proxyFuture.get() != NULL) {
                    return proxyFuture->get();
                }

                proxyFuture.reset(new util::Future<boost::shared_ptr<ClientProxy> >());
                boost::shared_ptr<util::Future<boost::shared_ptr<ClientProxy> > > current = proxies.putIfAbsent(ns,
                                                                                                                proxyFuture);
                if (current.get()) {
                    return current->get();
                }

                try {
                    boost::shared_ptr<ClientProxy> clientProxy = factory.create(id);
                    initializeWithRetry(clientProxy);
                    proxyFuture->set_value(clientProxy);
                    return clientProxy;
                } catch (exception::IException &e) {
                    proxies.remove(ns);
                    proxyFuture->set_exception(e.clone());
                    throw;
                }
            }

            void ProxyManager::initializeWithRetry(boost::shared_ptr<ClientProxy> clientProxy) {
                // TODO: Implement like java
                clientProxy->onInitialize();
            }
        }
    }
}
