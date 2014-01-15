//
// Created by sancar koyunlu on 20/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/semaphore/AcquireRequest.h"
#include "hazelcast/client/semaphore/SemaphorePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace semaphore {
            AcquireRequest::AcquireRequest(const std::string &instanceName, int permitCount, long timeout)
            : SemaphoreRequest(instanceName, permitCount)
            , timeout(timeout) {

            };

            int AcquireRequest::getClassId() const {
                return SemaphorePortableHook::ACQUIRE;
            };


            void AcquireRequest::write(serialization::PortableWriter &writer) const {
                SemaphoreRequest::write(writer);
                writer.writeLong("t", timeout);
            };

        }
    }
}

