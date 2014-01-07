//
// Created by sancar koyunlu on 12/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/proxy/DistributedObject.h"
#include "hazelcast/client/impl/ClientDestroyRequest.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/spi/ClusterService.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            DistributedObject::DistributedObject(const std::string &serviceName, const std::string &objectName, spi::ClientContext *context)
            : name(objectName), serviceName(serviceName), context(context) {

            }

            DistributedObject::~DistributedObject() {

            }

            const std::string &DistributedObject::getServiceName() const {
                return serviceName;
            }


            const std::string &DistributedObject::getName() const {
                return name;
            }

            spi::ClientContext &DistributedObject::getContext() {
                return *context;
            }

            void DistributedObject::destroy() {
                onDestroy();
                impl::ClientDestroyRequest request(name, serviceName);
                context->getInvocationService().invokeOnRandomTarget(request);
            }

            std::string DistributedObject::listen(const impl::PortableRequest &registrationRequest, const serialization::Data *partitionKey, impl::EventHandlerWrapper *handler) {
                boost::shared_future<serialization::Data> future;
                if (partitionKey == NULL) {
                    future = context->getInvocationService().invokeOnRandomTarget(registrationRequest, handler);
                } else {
                    future = context->getInvocationService().invokeOnKeyOwner(registrationRequest, handler, *partitionKey);
                }
                boost::shared_ptr<std::string> registrationId = context->getSerializationService().toObject<std::string>(future.get());
                context->getClusterService().registerListener(*registrationId, registrationRequest.callId);
                return *registrationId;
            }

            std::string DistributedObject::listen(const impl::PortableRequest &registrationRequest, impl::EventHandlerWrapper *handler) {
                return listen(registrationRequest, NULL, handler);
            }

            bool DistributedObject::stopListening(const impl::PortableRequest &request, const std::string &registrationId) {
                boost::shared_future<serialization::Data> future = context->getInvocationService().invokeOnRandomTarget(request);
                bool result = context->getSerializationService().toObject<bool>(future.get());
                context->getClusterService().deRegisterListener(registrationId);
                return result;
            }
        }
    }
}
