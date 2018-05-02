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

#include "hazelcast/util/UuidUtil.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/spi/impl/ListenerMessageCodec.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/util/Callable.h"
#include "hazelcast/client/spi/impl/listener/NonSmartClientListenerService.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace listener {

                    NonSmartClientListenerService::NonSmartClientListenerService(ClientContext &clientContext,
                                                                                 int32_t eventThreadCount,
                                                                                 int32_t eventQueueCapacity)
                            : AbstractClientListenerService(clientContext, eventThreadCount, eventQueueCapacity) {

                    }

                    void NonSmartClientListenerService::start() {
                        clientContext.getConnectionManager().addConnectionListener(shared_from_this());
                    }

                    std::string NonSmartClientListenerService::registerListener(
                            const boost::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                            const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        //This method should not be called from registrationExecutor
/*                      TODO
                        assert (!Thread.currentThread().getName().contains("eventRegistration"));
*/

                        boost::shared_ptr<util::Callable<std::string> > task(
                                new RegisterListenerTask("NonSmartClientListenerService::registerListener",
                                                         shared_from_this(), listenerMessageCodec, handler));

                        return registrationExecutor.submit<std::string>(task)->get();
                    }

                    bool NonSmartClientListenerService::deregisterListener(const std::string &registrationId) {
                        //This method should not be called from registrationExecutor
/*                      TODO
                        assert (!Thread.currentThread().getName().contains("eventRegistration"));
*/
                        boost::shared_ptr<util::Callable<bool> > task(
                                new DeregisterListenerTask("NonSmartClientListenerService::deregisterListener",
                                                           shared_from_this(), registrationId));

                        return registrationExecutor.submit<bool>(task)->get();
                    }

                    void NonSmartClientListenerService::connectionAdded(
                            const boost::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/
                        registrationExecutor.execute(
                                boost::shared_ptr<util::Runnable>(
                                        new ConnectionAddedTask("NonSmartClientListenerService::connectionAdded",
                                                                shared_from_this(), connection)));
                    }

                    void NonSmartClientListenerService::connectionRemoved(
                            const boost::shared_ptr<connection::Connection> &connection) {
                        // no-op
                    }

                    boost::shared_ptr<ClientEventRegistration>
                    NonSmartClientListenerService::invoke(const ClientRegistrationKey &registrationKey) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/

                        const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler = registrationKey.getHandler();
                        handler->beforeListenerRegister();
                        std::auto_ptr<protocol::ClientMessage> request = registrationKey.getCodec()->encodeAddRequest(
                                false);
                        boost::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(clientContext,
                                                                                                  request, "");
                        invocation->setEventHandler(handler);

                        boost::shared_ptr<ClientInvocationFuture> future = invocation->invoke();
                        std::string registrationId = registrationKey.getCodec()->decodeAddResponse(*future->get());
                        handler->onListenerRegister();
                        boost::shared_ptr<connection::Connection> connection = invocation->getSendConnection();
                        return boost::shared_ptr<ClientEventRegistration>(
                                new ClientEventRegistration(registrationId,
                                                            invocation->getClientMessage()->getCorrelationId(),
                                                            connection, registrationKey.getCodec()));

                    }

                    std::string NonSmartClientListenerService::registerListenerInternal(
                            const boost::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                            const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        std::string userRegistrationId = util::UuidUtil::newUnsecureUuidString();
                        ClientRegistrationKey registrationKey(userRegistrationId, handler, listenerMessageCodec);
                        try {
                            boost::shared_ptr<ClientEventRegistration> registration = invoke(registrationKey);
                            activeRegistrations.put(registrationKey, registration);
                            userRegistrations.insert(registrationKey);
                        } catch (exception::IException &e) {
                            throw (exception::ExceptionBuilder<exception::HazelcastException>(
                                    "NonSmartClientListenerService::registerListenerInternal")
                                    << "Listener can not be added. " << e).build();
                        }
                        return userRegistrationId;
                    }

                    bool
                    NonSmartClientListenerService::deregisterListenerInternal(const std::string &userRegistrationId) {
                        ClientRegistrationKey key(userRegistrationId);

                        if (!userRegistrations.erase(key)) {
                            return false;
                        }

                        boost::shared_ptr<ClientEventRegistration> registration = activeRegistrations.get(key);
                        if (registration.get() == NULL) {
                            return true;
                        }

                        std::auto_ptr<protocol::ClientMessage> request = registration->getCodec()->encodeRemoveRequest(
                                registration->getServerRegistrationId());
                        try {
                            boost::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(clientContext,
                                                                                                      request, "");
                            invocation->invoke()->get();
                            removeEventHandler(registration->getCallId());
                            activeRegistrations.remove(key);
                        } catch (exception::IException &e) {
                            throw (exception::ExceptionBuilder<exception::HazelcastException>(
                                    "NonSmartClientListenerService::deregisterListenerInternal") << "Listener with ID "
                                                                                                 << userRegistrationId
                                                                                                 << " could not be removed. Cause:"
                                                                                                 << e).build();
                        }
                        return true;
                    }

                    void NonSmartClientListenerService::connectionAddedInternal(
                            const boost::shared_ptr<connection::Connection> &connection) {
                        BOOST_FOREACH (const ClientRegistrationKey &registrationKey, userRegistrations) {
                                        try {
                                            boost::shared_ptr<ClientEventRegistration> oldRegistration = activeRegistrations.get(
                                                    registrationKey);
                                            if (oldRegistration.get() != NULL) {
                                                //if there was a registration corresponding to same user listener key before,
                                                //then we need to remove its event handler as cleanup
                                                removeEventHandler(oldRegistration->getCallId());
                                            }
                                            boost::shared_ptr<ClientEventRegistration> eventRegistration = invoke(
                                                    registrationKey);
                                            activeRegistrations.put(registrationKey, eventRegistration);
                                        } catch (exception::IException &e) {
                                            logger.warning() << "Listener " << registrationKey
                                                             << " can not be added to new connection: "
                                                             << *connection << " Cause:" << e;
                                        }
                                    }

                    }

                }
            }
        }

    }
}
