//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/connection/CallPromise.h"
#include "hazelcast/client/connection/CallFuture.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/impl/BaseRemoveListenerRequest.h"
#include "hazelcast/client/impl/BaseEventHandler.h"

namespace hazelcast {
    namespace client {
        namespace spi {

            ServerListenerService::ServerListenerService(spi::ClientContext& clientContext)
            : clientContext(clientContext) {

            }

            std::string ServerListenerService::listen(const impl::ClientRequest *registrationRequest, int partitionId, impl::BaseEventHandler *handler) {
                connection::CallFuture future = clientContext.getInvocationService().invokeOnPartitionOwner(registrationRequest, handler, partitionId);
                boost::shared_ptr<std::string> registrationId = clientContext.getSerializationService().toObject<std::string>(future.get());
                handler->registrationId = *registrationId;
                registerListener(registrationId, registrationRequest->callId);

                return *registrationId;
            }

            std::string ServerListenerService::listen(const impl::ClientRequest *registrationRequest, impl::BaseEventHandler *handler) {
                connection::CallFuture future = clientContext.getInvocationService().invokeOnRandomTarget(registrationRequest, handler);
                boost::shared_ptr<std::string> registrationId = clientContext.getSerializationService().toObject<std::string>(future.get());
                handler->registrationId = *registrationId;
                registerListener(registrationId, registrationRequest->callId);

                return *registrationId;
            }

            bool ServerListenerService::stopListening(impl::BaseRemoveListenerRequest *request, const std::string& registrationId) {
                std::string resolvedRegistrationId = registrationId;
                bool isValidId = deRegisterListener(resolvedRegistrationId);
                if (!isValidId) {
                    delete request;
                    return false;
                }
                request->setRegistrationId(resolvedRegistrationId);
                connection::CallFuture future = clientContext.getInvocationService().invokeOnRandomTarget(request);
                bool result = *(clientContext.getSerializationService().toObject<bool>(future.get()));
                return result;
            }

            void ServerListenerService::registerListener(boost::shared_ptr<std::string> registrationId, int callId) {
                registrationAliasMap.put(*registrationId, registrationId);
                registrationIdMap.put(*registrationId, boost::shared_ptr<int>(new int(callId)));
            }

            void ServerListenerService::reRegisterListener(const std::string& registrationId, boost::shared_ptr<std::string> alias, int callId) {
                boost::shared_ptr<const std::string> oldAlias = registrationAliasMap.put(registrationId, alias);
                if (oldAlias.get() != NULL) {
                    registrationIdMap.remove(*oldAlias);
                    registrationIdMap.put(*alias, boost::shared_ptr<int>(new int(callId)));
                }
            }

            bool ServerListenerService::deRegisterListener(std::string& registrationId) {
                boost::shared_ptr<const std::string> uuid = registrationAliasMap.remove(registrationId);
                if (uuid != NULL) {
                    registrationId = *uuid;
                    boost::shared_ptr<int> callId = registrationIdMap.remove(*uuid);
                    clientContext.getInvocationService().removeEventHandler(*callId);
                    return true;
                }
                return false;
            }

            void ServerListenerService::retryFailedListener(boost::shared_ptr<connection::CallPromise> listenerPromise) {
                util::LockGuard lockGuard(failedListenerLock);
                // Just put into the list, it shall be retried when the cluster listener thread reconnects to another node
                // Not: If a resend is performed here retrying the listener promise, the cluster listener thread shall
                // be blocked on connection close.
                failedListeners.push_back(listenerPromise);
            }

            void ServerListenerService::triggerFailedListeners() {
                std::vector<boost::shared_ptr<connection::CallPromise> >::iterator it;
                std::vector<boost::shared_ptr<connection::CallPromise> > newFailedListeners;
                InvocationService& invocationService = clientContext.getInvocationService();

                util::LockGuard lockGuard(failedListenerLock);
                for (it = failedListeners.begin(); it != failedListeners.end(); ++it) {
                    try {
                        boost::shared_ptr<connection::Connection> result = invocationService.resend(*it, "internalRetryOfUnkownAddress");

                        if (NULL == result.get()) { // resend failed
                            newFailedListeners.push_back(*it);
                        }
                    } catch (exception::IOException&) {
                        newFailedListeners.push_back(*it);
                        continue;
                    }
                }
                failedListeners = newFailedListeners;
            }

        }
    }
}

