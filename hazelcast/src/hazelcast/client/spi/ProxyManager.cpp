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
#include "hazelcast/client/spi/impl/AbstractClientInvocationService.h"
#include "hazelcast/client/protocol/codec/ClientCreateProxyCodec.h"
#include "hazelcast/client/spi/ClientProxy.h"
#include "hazelcast/client/spi/ClientProxyFactory.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/ClientConfig.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            ProxyManager::ProxyManager(ClientContext &context) : client(context) {
            }

            void ProxyManager::init() {
                spi::impl::AbstractClientInvocationService &invocationService =
                        (spi::impl::AbstractClientInvocationService &) client.getInvocationService();

                invocationTimeoutMillis = invocationService.getInvocationTimeoutMillis();
                invocationRetryPauseMillis = invocationService.getInvocationRetryPauseMillis();
            }

            boost::shared_ptr<ClientProxy> ProxyManager::getOrCreateProxy(
                    const std::string &service, const std::string &id, ClientProxyFactory &factory) {
                DefaultObjectNamespace ns(service, id);
                boost::shared_ptr<util::Future<ClientProxy> > proxyFuture = proxies.get(ns);
                if (proxyFuture.get() != NULL) {
                    return proxyFuture->get();
                }

                proxyFuture.reset(new util::Future<ClientProxy>(client.getLogger()));
                boost::shared_ptr<util::Future<ClientProxy> > current = proxies.putIfAbsent(ns, proxyFuture);
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

            void ProxyManager::initializeWithRetry(const boost::shared_ptr<ClientProxy> &clientProxy) {
                int64_t startMillis = util::currentTimeMillis();
                while (util::currentTimeMillis() < startMillis + invocationTimeoutMillis) {
                    try {
                        initialize(clientProxy);
                        return;
                    } catch (exception::IException &e) {
                        bool retryable = isRetryable(e);

                        if (!retryable) {
                            try {
                                e.raise();
                            } catch (exception::ExecutionException &) {
                                retryable = isRetryable(*e.getCause());
                            }
                        }

                        if (retryable) {
                            sleepForProxyInitRetry();
                        } else {
                            e.raise();
                        }
                    }
                }
                int64_t elapsedTime = util::currentTimeMillis() - startMillis;
                throw (exception::ExceptionBuilder<exception::OperationTimeoutException>(
                        "ProxyManager::initializeWithRetry") << "Initializing  " << clientProxy->getServiceName() << ":"
                                                             << clientProxy->getName() << " is timed out after "
                                                             << elapsedTime << " ms. Configured invocation timeout is "
                                                             << invocationTimeoutMillis << " ms").build();
            }

            void ProxyManager::initialize(const boost::shared_ptr<ClientProxy> &clientProxy) {
                boost::shared_ptr<Address> initializationTarget = findNextAddressToSendCreateRequest();
                if (initializationTarget.get() == NULL) {
                    throw exception::IOException("ProxyManager::initialize",
                                                 "Not able to find a member to create proxy on!");
                }
                std::auto_ptr<protocol::ClientMessage> clientMessage = protocol::codec::ClientCreateProxyCodec::encodeRequest(
                        clientProxy->getName(),
                        clientProxy->getServiceName(), *initializationTarget);
                spi::impl::ClientInvocation::create(client, clientMessage, clientProxy->getServiceName(),
                                                    *initializationTarget)->invoke()->get();
                clientProxy->onInitialize();
            }

            boost::shared_ptr<Address> ProxyManager::findNextAddressToSendCreateRequest() {
                int clusterSize = client.getClientClusterService().getSize();
                if (clusterSize == 0) {
                    throw exception::HazelcastClientOfflineException("ProxyManager::findNextAddressToSendCreateRequest",
                                                                     "Client connecting to cluster");
                }
                boost::shared_ptr<Member> liteMember;

                LoadBalancer *loadBalancer = client.getClientConfig().getLoadBalancer();
                for (int i = 0; i < clusterSize; i++) {
                    boost::shared_ptr<Member> member;
                    try {
                        member = boost::shared_ptr<Member>(new Member(loadBalancer->next()));
                    } catch (exception::IllegalStateException &) {
                        // skip
                    }
                    if (member.get() != NULL && !member->isLiteMember()) {
                        return boost::shared_ptr<Address>(new Address(member->getAddress()));
                    } else if (liteMember.get() == NULL) {
                        liteMember = member;
                    }
                }

                return liteMember.get() != NULL ? boost::shared_ptr<Address>(new Address(liteMember->getAddress()))
                                                : boost::shared_ptr<Address>();
            }

            bool ProxyManager::isRetryable(exception::IException &exception) {
                return spi::impl::ClientInvocation::isRetrySafeException(exception);
            }

            void ProxyManager::sleepForProxyInitRetry() {
                // TODO: change to interruptible sleep
                util::sleepmillis(invocationRetryPauseMillis);
            }

            void ProxyManager::destroyProxy(ClientProxy &proxy) {
                DefaultObjectNamespace objectNamespace(proxy.getServiceName(), proxy.getName());
                boost::shared_ptr<util::Future<ClientProxy> > registeredProxyFuture = proxies.remove(
                        objectNamespace);
                boost::shared_ptr<ClientProxy> registeredProxy;
                if (registeredProxyFuture.get()) {
                    registeredProxy = registeredProxyFuture->get();
                }

                try {
                    if (registeredProxy.get() != NULL) {
                        try {
                            registeredProxy->destroyLocally();
                            registeredProxy->destroyRemotely();
                        } catch (exception::IException &e) {
                            registeredProxy->destroyRemotely();
                            e.raise();
                        }
                    }
                    if (&proxy != registeredProxy.get()) {
                        // The given proxy is stale and was already destroyed, but the caller
                        // may have allocated local resources in the context of this stale proxy
                        // instance after it was destroyed, so we have to cleanup it locally one
                        // more time to make sure there are no leaking local resources.
                        proxy.destroyLocally();
                    }
                } catch (exception::IException &e) {
                    if (&proxy != registeredProxy.get()) {
                        // The given proxy is stale and was already destroyed, but the caller
                        // may have allocated local resources in the context of this stale proxy
                        // instance after it was destroyed, so we have to cleanup it locally one
                        // more time to make sure there are no leaking local resources.
                        proxy.destroyLocally();
                    }
                    e.raise();
                }

            }

        }
    }
}
