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

#include <hazelcast/client/spi/ClientProxy.h>

#include "hazelcast/client/spi/ClientProxy.h"
#include "hazelcast/client/protocol/codec/ClientDestroyProxyCodec.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/ProxyManager.h"
#include "hazelcast/client/spi/ClientListenerService.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            ClientProxy::ClientProxy(const std::string &name, const std::string &serviceName, ClientContext &context)
                    : name(name), serviceName(serviceName), context(context) {}

            ClientProxy::~ClientProxy() {}

            const std::string &ClientProxy::getName() const {
                return name;
            }

            const std::string &ClientProxy::getServiceName() const {
                return serviceName;
            }

            ClientContext &ClientProxy::getContext() {
                return context;
            }

            void ClientProxy::onDestroy() {
            }

            void ClientProxy::destroy() {
                getContext().getProxyManager().destroyProxy(*this);
            }

            void ClientProxy::destroyLocally() {
                if (preDestroy()) {
                    try {
                        onDestroy();
                        postDestroy();
                    } catch (exception::IException &e) {
                        postDestroy();
                        e.raise();
                    }
                }
            }

            bool ClientProxy::preDestroy() {
                return true;
            }

            void ClientProxy::postDestroy() {
            }

            void ClientProxy::onInitialize() {
            }

            void ClientProxy::onShutdown() {
            }

            serialization::pimpl::SerializationService &ClientProxy::getSerializationService() {
                return context.getSerializationService();
            }

            void ClientProxy::destroyRemotely() {
                std::auto_ptr<protocol::ClientMessage> clientMessage = protocol::codec::ClientDestroyProxyCodec::encodeRequest(
                        getName(), getServiceName());
                spi::impl::ClientInvocation::create(getContext(), clientMessage, getName())->invoke()->get();
            }

            ClientProxy::EventHandlerDelegator::EventHandlerDelegator(client::impl::BaseEventHandler *handler) : handler(
                    handler) {}

            void ClientProxy::EventHandlerDelegator::handle(const boost::shared_ptr<protocol::ClientMessage> &event) {
                handler->handle(event);
            }

            void ClientProxy::EventHandlerDelegator::beforeListenerRegister() {
                handler->beforeListenerRegister();
            }

            void ClientProxy::EventHandlerDelegator::onListenerRegister() {
                handler->onListenerRegister();
            }

            std::string ClientProxy::registerListener(const boost::shared_ptr<spi::impl::ListenerMessageCodec> &codec,
                                                    client::impl::BaseEventHandler *handler) {
                handler->setLogger(&getContext().getLogger());
                return getContext().getClientListenerService().registerListener(codec,
                                                                                boost::shared_ptr<spi::EventHandler<protocol::ClientMessage> >(
                                                                                        new EventHandlerDelegator(
                                                                                                handler)));
            }

            std::string
            ClientProxy::registerListener(const boost::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                                          const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                boost::static_pointer_cast<client::impl::BaseEventHandler>(handler)->setLogger(&getContext().getLogger());
                return getContext().getClientListenerService().registerListener(listenerMessageCodec, handler);
            }

            bool ClientProxy::deregisterListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }
        }
    }
}


