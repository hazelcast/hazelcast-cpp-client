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
#include "hazelcast/client/impl/BaseRemoveListenerRequest.h"

namespace hazelcast {
    namespace client {
        namespace spi {

            ServerListenerService::ServerListenerService(spi::ClientContext &clientContext)
            :clientContext(clientContext) {

            };

            std::string ServerListenerService::listen(const impl::PortableRequest *registrationRequest, int partitionId, impl::BaseEventHandler *handler) {
                boost::shared_future<serialization::Data> future;
                future = clientContext.getInvocationService().invokeOnKeyOwner(registrationRequest, handler, partitionId);
                boost::shared_ptr<std::string> registrationId = clientContext.getSerializationService().toObject<std::string>(future.get());
                handler->registrationId = *registrationId;
                registerListener(registrationId, registrationRequest->callId);

                return *registrationId;
            }

            std::string ServerListenerService::listen(const impl::PortableRequest *registrationRequest, impl::BaseEventHandler *handler) {
                boost::shared_future<serialization::Data> future;
                future = clientContext.getInvocationService().invokeOnRandomTarget(registrationRequest, handler);
                boost::shared_ptr<std::string> registrationId = clientContext.getSerializationService().toObject<std::string>(future.get());
                handler->registrationId = *registrationId;
                registerListener(registrationId, registrationRequest->callId);

                return *registrationId;
            }

            bool ServerListenerService::stopListening(impl::BaseRemoveListenerRequest *request, const std::string &registrationId) {
                std::string resolvedRegistrationId = registrationId;
                bool notRegistered = deRegisterListener(resolvedRegistrationId);
                if(notRegistered)
                    return false;
                request->setRegistrationId(resolvedRegistrationId);
                boost::shared_future<serialization::Data> future = clientContext.getInvocationService().invokeOnRandomTarget(request);
                bool result = clientContext.getSerializationService().toObject<bool>(future.get());
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
        }
    }
}
