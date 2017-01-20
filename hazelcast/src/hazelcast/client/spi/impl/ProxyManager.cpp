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

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/ClientProxy.h"
#include "hazelcast/client/spi/ClientProxyFactory.h"
#include "hazelcast/client/exception/IException.h"
#include "hazelcast/client/spi/ProxyManager.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            ProxyManager::ProxyManager(ClientContext &context) {
            }

            boost::shared_ptr<ClientProxy> ProxyManager::getOrCreateProxy(
                    const std::string &service, const std::string &id, ClientProxyFactory &factory) {
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

            void ProxyManager::initializeWithRetry(boost::shared_ptr<ClientProxy> clientProxy) {
                // TODO: Implement like java
                clientProxy->onInitialize();
            }

        }
    }
}
