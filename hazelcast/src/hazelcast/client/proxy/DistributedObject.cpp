//
// Created by sancar koyunlu on 12/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/proxy/DistributedObject.h"
#include "hazelcast/client/impl/ClientDestroyRequest.h"
#include "hazelcast/client/spi/InvocationService.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            DistributedObject::DistributedObject(const std::string &serviceName, const std::string &objectName, spi::ClientContext *context)
            : name(objectName), serviceName(serviceName), context(context) {

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
                context->getInvocationService().invokeOnRandomTarget<bool>(request);
            }

            DistributedObject::~DistributedObject(){

            }
        }
    }
}
