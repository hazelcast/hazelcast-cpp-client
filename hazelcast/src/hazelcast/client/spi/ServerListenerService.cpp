//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/impl/BaseRemoveListenerRequest.h"

namespace hazelcast {
    namespace client {
        namespace spi {

            ServerListenerService::ServerListenerService(spi::ClientContext &clientContext)
            :clientContext(clientContext) {

            }

            std::string ServerListenerService::listen(const impl::ClientRequest*registrationRequest, int partitionId, impl::BaseEventHandler *handler) {
                boost::shared_ptr< util::Future<serialization::pimpl::Data> >  future;
                future = clientContext.getInvocationService().invokeOnKeyOwner(registrationRequest, handler, partitionId);
                boost::shared_ptr<std::string> registrationId = clientContext.getSerializationService().toObject<std::string>(future->get());
                handler->registrationId = *registrationId;
                registerListener(registrationId, registrationRequest->callId);

                return *registrationId;
            }

            std::string ServerListenerService::listen(const impl::ClientRequest*registrationRequest, impl::BaseEventHandler *handler) {
                boost::shared_ptr< util::Future<serialization::pimpl::Data> >  future;
                future = clientContext.getInvocationService().invokeOnRandomTarget(registrationRequest, handler);
                boost::shared_ptr<std::string> registrationId = clientContext.getSerializationService().toObject<std::string>(future->get());
                handler->registrationId = *registrationId;
                registerListener(registrationId, registrationRequest->callId);

                return *registrationId;
            }

            bool ServerListenerService::stopListening(impl::BaseRemoveListenerRequest *request, const std::string &registrationId) {
                std::string resolvedRegistrationId = registrationId;
                bool isValidId = deRegisterListener(resolvedRegistrationId);
                if (!isValidId) {
                    delete request;
                    return false;
                }
                request->setRegistrationId(resolvedRegistrationId);
                boost::shared_ptr< util::Future<serialization::pimpl::Data> >  future = clientContext.getInvocationService().invokeOnRandomTarget(request);
                bool result = *(clientContext.getSerializationService().toObject<bool>(future->get()));
                return result;
            }

            void ServerListenerService::registerListener(boost::shared_ptr<std::string> registrationId, int callId) {
                registrationAliasMap.put(*registrationId, registrationId);
                registrationIdMap.put(*registrationId, boost::shared_ptr<int>(new int(callId)));
            }

            void ServerListenerService::reRegisterListener(const std::string &registrationId, boost::shared_ptr<std::string> alias, int callId) {
                boost::shared_ptr<const std::string> oldAlias = registrationAliasMap.put(registrationId, alias);
                if (oldAlias.get() != NULL) {
                    registrationIdMap.remove(*oldAlias);
                    registrationIdMap.put(*alias, boost::shared_ptr<int>(new int(callId)));
                }
            }

            bool ServerListenerService::deRegisterListener(std::string &registrationId) {
                boost::shared_ptr<const std::string> uuid = registrationAliasMap.remove(registrationId);
                if (uuid != NULL) {
                    registrationId = *uuid;
                    boost::shared_ptr<int> callId = registrationIdMap.remove(*uuid);
                    clientContext.getConnectionManager().removeEventHandler(*callId);
                    return true;
                }
                return false;
            }

            void ServerListenerService::retryFailedListener(boost::shared_ptr<connection::CallPromise> failedListener) {
                boost::shared_ptr<connection::Connection> connection;
                try {
                    connection::ConnectionManager &cm = clientContext.getConnectionManager();
                    connection = cm.getRandomConnection(spi::InvocationService::RETRY_COUNT);
                } catch(exception::IException &) {
                    util::LockGuard lockGuard(failedListenerLock);
                    failedListeners.push_back(failedListener);
                    return;
                }
                connection->registerAndEnqueue(failedListener, -1);
            }

            void ServerListenerService::triggerFailedListeners() {
                std::vector< boost::shared_ptr<connection::CallPromise> >::iterator it;
                std::vector< boost::shared_ptr<connection::CallPromise> > newFailedListeners;

                util::LockGuard lockGuard(failedListenerLock);
                for (it = failedListeners.begin(); it != failedListeners.end(); ++it) {
                    boost::shared_ptr<connection::Connection> connection;
                    try {
                        connection::ConnectionManager &cm = clientContext.getConnectionManager();
                        connection = cm.getRandomConnection(spi::InvocationService::RETRY_COUNT);
                    } catch(exception::IOException &) {
                        newFailedListeners.push_back(*it);
                        continue;
                    }
                    connection->registerAndEnqueue(*it, -1);
                }
                failedListeners = newFailedListeners;
            }
        }
    }
}

