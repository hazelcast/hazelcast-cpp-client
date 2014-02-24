//
// Created by sancar koyunlu on 12/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/impl/ClientDestroyRequest.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/impl/BaseEventHandler.h"

namespace hazelcast {
    namespace client {
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
            impl::ClientDestroyRequest *request = new impl::ClientDestroyRequest(name, serviceName);
            context->getInvocationService().invokeOnRandomTarget(request);
        }

        std::string DistributedObject::listen(const impl::PortableRequest *registrationRequest, int partitionId, impl::BaseEventHandler *handler) {
            return context->getServerListenerService().listen(registrationRequest, partitionId, handler);
        }

        std::string DistributedObject::listen(const impl::PortableRequest *registrationRequest, impl::BaseEventHandler *handler) {
            return context->getServerListenerService().listen(registrationRequest, handler);
        }

        bool DistributedObject::stopListening(impl::BaseRemoveListenerRequest *request, const std::string &registrationId) {
            return context->getServerListenerService().stopListening(request, registrationId);
        }

        int DistributedObject::getPartitionId(const serialization::pimpl::Data &key) {
            return context->getPartitionService().getPartitionId(key);
        }
    }
}
