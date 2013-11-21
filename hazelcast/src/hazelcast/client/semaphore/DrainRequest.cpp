//
// Created by sancar koyunlu on 20/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/semaphore/DrainRequest.h"
#include "hazelcast/client/semaphore/SemaphorePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace semaphore {
            DrainRequest::DrainRequest(const std::string& instanceName)
                : SemaphoreRequest(instanceName, -1) {

                };

                int DrainRequest::getClassId() const {
                    return SemaphorePortableHook::DRAIN;
                };


                void DrainRequest::writePortable(serialization::PortableWriter& writer) const {
                    SemaphoreRequest::writePortable(writer);
                };
        }
    }
}


