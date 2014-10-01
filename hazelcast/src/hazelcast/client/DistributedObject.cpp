//
// Created by sancar koyunlu on 12/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

namespace hazelcast {
    namespace client {
        DistributedObject::DistributedObject(const std::string& serviceName, const std::string& objectName)
        : name(objectName), serviceName(serviceName) {

        }

        DistributedObject::~DistributedObject() {

        }

        const std::string& DistributedObject::getServiceName() const {
            return serviceName;
        }


        const std::string& DistributedObject::getName() const {
            return name;
        }


        void DistributedObject::onDestroy() {

        }
    }
}

