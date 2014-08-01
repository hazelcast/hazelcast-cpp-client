//
// Created by sancar koyunlu on 20/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/semaphore/ReleaseRequest.h"
#include "hazelcast/client/semaphore/SemaphorePortableHook.h"

namespace hazelcast {
    namespace client {
        namespace semaphore {
            ReleaseRequest::ReleaseRequest(const std::string &instanceName, int permitCount)
            : SemaphoreRequest(instanceName, permitCount) {

            }

            int ReleaseRequest::getClassId() const {
                return SemaphorePortableHook::RELEASE;
            }
        }
    }
}
