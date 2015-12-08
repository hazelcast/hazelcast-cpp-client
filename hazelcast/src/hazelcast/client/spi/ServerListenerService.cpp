/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 6/24/13.

#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/connection/CallPromise.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/protocol/codec/IAddListenerCodec.h"
#include "hazelcast/client/protocol/codec/IRemoveListenerCodec.h"
#include "hazelcast/client/spi/impl/listener/EventRegistration.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            ServerListenerService::ServerListenerService(spi::ClientContext &clientContext)
                    : clientContext(clientContext) {
            }

            std::string ServerListenerService::registerListener(
                    std::auto_ptr<protocol::codec::IAddListenerCodec> addListenerCodec,
                    int partitionId, client::impl::BaseEventHandler *handler) {

                std::auto_ptr<protocol::ClientMessage> request = addListenerCodec->encodeRequest();

                connection::CallFuture future = clientContext.getInvocationService().invokeOnPartitionOwner(
                        request, handler, partitionId);

                return registerInternal(addListenerCodec, future);
            }

            std::string ServerListenerService::registerListener(
                    std::auto_ptr<protocol::codec::IAddListenerCodec> addListenerCodec,
                    client::impl::BaseEventHandler *handler) {
                connection::CallFuture future = clientContext.getInvocationService().invokeOnRandomTarget(
                        addListenerCodec->encodeRequest(), handler);

                return registerInternal(addListenerCodec, future);
            }

            void ServerListenerService::reRegisterListener(std::string registrationId,
                                                           protocol::ClientMessage *response) {

                boost::shared_ptr<impl::listener::EventRegistration> registration = registrationIdMap.get(
                        registrationId);
                if (NULL != registration.get()) {
                    // registration exists, just change the alias
                    boost::shared_ptr<std::string> alias(
                            new std::string(registration->getAddCodec()->decodeResponse(*response)));
                    boost::shared_ptr<const std::string> oldAlias = registrationAliasMap.put(registrationId, alias);
                    if (oldAlias.get() != NULL) {
                        registrationIdMap.remove(*oldAlias);
                        registration->setCorrelationId(response->getCorrelationId());
                        registrationIdMap.put(*alias, registration);
                    }
                }
            }

            bool ServerListenerService::deRegisterListener(protocol::codec::IRemoveListenerCodec &removeListenerCodec) {
                boost::shared_ptr<const std::string> uuid = registrationAliasMap.remove(removeListenerCodec.getRegistrationId());
                if (uuid != NULL) {
                    boost::shared_ptr<impl::listener::EventRegistration> registration = registrationIdMap.remove(*uuid);

                    clientContext.getInvocationService().removeEventHandler(registration->getCorrelationId());

                    // send a remove listener request
                    removeListenerCodec.setRegistrationId(*uuid);
                    std::auto_ptr<protocol::ClientMessage> request = removeListenerCodec.encodeRequest();
                    try {
                        connection::CallFuture future = clientContext.getInvocationService().invokeOnTarget(
                                request, registration->getMemberAddress());

                        std::auto_ptr<protocol::ClientMessage> response = future.get();
                    } catch (exception::IException &e) {
                        //if invocation cannot be done that means connection is broken and listener is already removed
						(void)e; // suppress the unused variable warning
                    }

                    return true;
                }
                return false;
            }

            void ServerListenerService::retryFailedListener(
                    boost::shared_ptr<connection::CallPromise> listenerPromise) {
                try {
                    InvocationService &invocationService = clientContext.getInvocationService();
                    boost::shared_ptr<connection::Connection> result =
                            invocationService.resend(listenerPromise, "internalRetryOfUnkownAddress");
                    if (NULL == result.get()) {
                        util::LockGuard lockGuard(failedListenerLock);
                        failedListeners.push_back(listenerPromise);
                    }
                } catch (exception::IException &) {
                    util::LockGuard lockGuard(failedListenerLock);
                    failedListeners.push_back(listenerPromise);
                }
            }

            void ServerListenerService::triggerFailedListeners() {
                std::vector<boost::shared_ptr<connection::CallPromise> >::iterator it;
                std::vector<boost::shared_ptr<connection::CallPromise> > newFailedListeners;
                InvocationService &invocationService = clientContext.getInvocationService();

                util::LockGuard lockGuard(failedListenerLock);
                for (it = failedListeners.begin(); it != failedListeners.end(); ++it) {
                    try {
                        boost::shared_ptr<connection::Connection> result =
                                invocationService.resend(*it, "internalRetryOfUnkownAddress");

                        if (NULL == result.get()) { // resend failed
                            newFailedListeners.push_back(*it);
                        }
                    } catch (exception::IOException &) {
                        newFailedListeners.push_back(*it);
                        continue;
                    }
                }
                failedListeners = newFailedListeners;
            }

            std::string ServerListenerService::registerInternal(
                    std::auto_ptr<protocol::codec::IAddListenerCodec> &addListenerCodec,
                    connection::CallFuture &future) {
                std::auto_ptr<protocol::ClientMessage> response = future.get();

                // get the correlationId for the request
                int correlationId = future.getCallId();

                std::string registrationId = addListenerCodec->decodeResponse(*response);

                registrationAliasMap.put(registrationId,
                                         boost::shared_ptr<std::string>(new std::string(registrationId)));

                registrationIdMap.put(registrationId, boost::shared_ptr<spi::impl::listener::EventRegistration>(
                        new spi::impl::listener::EventRegistration(correlationId,
                                                                   future.getConnection().getRemoteEndpoint(),
                                                                   addListenerCodec)));

                return registrationId;
            }

        }
    }
}

