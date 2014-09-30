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
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/connection/CallFuture.h"


namespace hazelcast {
    namespace client {
        DistributedObject::DistributedObject(const std::string &serviceName, const std::string &objectName, spi::ClientContext *context)
        : context(context) , name(objectName), serviceName(serviceName){

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

        std::string DistributedObject::listen(const impl::ClientRequest*registrationRequest, int partitionId, impl::BaseEventHandler *handler) {
            return context->getServerListenerService().listen(registrationRequest, partitionId, handler);
        }

        std::string DistributedObject::listen(const impl::ClientRequest*registrationRequest, impl::BaseEventHandler *handler) {
            return context->getServerListenerService().listen(registrationRequest, handler);
        }

        bool DistributedObject::stopListening(impl::BaseRemoveListenerRequest *request, const std::string &registrationId) {
            return context->getServerListenerService().stopListening(request, registrationId);
        }

        int DistributedObject::getPartitionId(const serialization::pimpl::Data &key) {
            return context->getPartitionService().getPartitionId(key);
        }

        serialization::pimpl::Data DistributedObject::invoke(const impl::ClientRequest *request, int partitionId) {
            spi::InvocationService &invocationService = getContext().getInvocationService();
            connection::CallFuture future = invocationService.invokeOnPartitionOwner(request, partitionId);
            return future.get();
        };

        serialization::pimpl::Data DistributedObject::invoke(const impl::ClientRequest *request) {
            connection::CallFuture  future = getContext().getInvocationService().invokeOnRandomTarget(request);
            return future.get();
        };
    }
}

