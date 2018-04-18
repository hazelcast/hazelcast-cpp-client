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
#include "hazelcast/util/Callable.h"
#include "hazelcast/client/spi/impl/listener/NonSmartClientListenerService.h"

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

                    std::string NonSmartClientListenerService::registerListener(
                            const boost::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                            const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        //This method should not be called from registrationExecutor
/*                      TODO
                        assert (!Thread.currentThread().getName().contains("eventRegistration"));
*/

                        boost::shared_ptr<util::Callable<std::string> > task(
                                new RegisterListenerTask(activeRegistrations, userRegistrations, listenerMessageCodec,
                                                         handler, *this));

                        return registrationExecutor.submit<std::string>(task)->get();
                    }

                    bool NonSmartClientListenerService::deregisterListener(const std::string &registrationId) {
                        return false;
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

                        boost::shared_ptr<ClientInvocationFuture> future = ClientInvocation::invoke(invocation);
                        std::string registrationId = registrationKey.getCodec()->decodeAddResponse(*future->get());
                        handler->onListenerRegister();
                        boost::shared_ptr<connection::Connection> connection = future->getInvocation()->getSendConnection();
                        return boost::shared_ptr<ClientEventRegistration>(new ClientEventRegistration(registrationId, request->getCorrelationId(), connection,
                                                       registrationKey.getCodec()));

                    }

                    NonSmartClientListenerService::RegisterListenerTask::RegisterListenerTask(
                            NonSmartClientListenerService::RegistrationsMap &activeRegistrations,
                            std::set<ClientRegistrationKey> &userRegistrations,
                            const boost::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                            const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler,
                            NonSmartClientListenerService &listenerService) : activeRegistrations(activeRegistrations),
                                                                              userRegistrations(userRegistrations),
                                                                              listenerMessageCodec(
                                                                                      listenerMessageCodec),
                                                                              handler(handler),
                                                                              listenerService(listenerService) {
                    }

                    std::string NonSmartClientListenerService::RegisterListenerTask::call() {
                        std::string userRegistrationId = util::UuidUtil::newUnsecureUuidString();
                        ClientRegistrationKey registrationKey(userRegistrationId, handler, listenerMessageCodec);
                        try {
                            boost::shared_ptr<ClientEventRegistration> registration = listenerService.invoke(
                                                                registrationKey);
                            activeRegistrations.put(registrationKey, registration);
                            userRegistrations.insert(registrationKey);
                        } catch (exception::IException &e) {
                            throw (exception::ExceptionBuilder<exception::HazelcastException>(
                                    "NonSmartClientListenerService::RegisterListenerTask::call")
                                    << "Listener can not be added. " << e).build();
                        }
                        return userRegistrationId;
                    }

                    const std::string NonSmartClientListenerService::RegisterListenerTask::getName() const {
                        return "NonSmartClientListenerService::RegisterListenerTask";
                    }
                }
            }
        }

    }
}
