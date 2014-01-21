//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/spi/ClientContext.h"

namespace hazelcast {
    namespace client {
        namespace spi {

            ServerListenerService::ServerListenerService(spi::ClientContext &clientContext)
            :clientContext(clientContext) {

            };

            std::string ServerListenerService::listen(const impl::PortableRequest *registrationRequest, const serialization::Data *partitionKey, impl::BaseEventHandler *handler) {
                boost::shared_future<serialization::Data> future;
                if (partitionKey == NULL) {
                    future = clientContext.getInvocationService().invokeOnRandomTarget(registrationRequest, handler);
                } else {
                    future = clientContext.getInvocationService().invokeOnKeyOwner(registrationRequest, handler, *partitionKey);
                }
                boost::shared_ptr<std::string> registrationId = clientContext.getSerializationService().toObject<std::string>(future.get());
                handler->registrationId = *registrationId;
                registerListener(registrationId, registrationRequest->callId);

                return *registrationId;
            }

            std::string ServerListenerService::listen(const impl::PortableRequest *registrationRequest, impl::BaseEventHandler *handler) {
                return listen(registrationRequest, NULL, handler);
            }

            bool ServerListenerService::stopListening(const impl::PortableRequest *request, const std::string &registrationId) {
                boost::shared_future<serialization::Data> future = clientContext.getInvocationService().invokeOnRandomTarget(request);
                bool result = clientContext.getSerializationService().toObject<bool>(future.get());
                deRegisterListener(registrationId);
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

            bool ServerListenerService::deRegisterListener(const std::string &registrationId) {
                boost::shared_ptr<const std::string> alias = registrationAliasMap.remove(registrationId);
                if (alias != NULL) {
                    boost::shared_ptr<int> callId = registrationIdMap.remove(*alias);
                    clientContext.getConnectionManager().removeEventHandler(*callId);
                    return true;
                }
                return false;
            }
        }
    }
}
