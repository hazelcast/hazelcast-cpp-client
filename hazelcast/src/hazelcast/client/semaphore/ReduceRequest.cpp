//
// Created by sancar koyunlu on 20/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/semaphore/ReduceRequest.h"
#include "hazelcast/client/semaphore/SemaphorePortableHook.h"

namespace hazelcast {
    namespace client {
        namespace semaphore {
            ReduceRequest::ReduceRequest(const std::string &instanceName, int permitCount)
            : SemaphoreRequest(instanceName, permitCount) {

            };

            int ReduceRequest::getClassId() const {
                return SemaphorePortableHook::REDUCE;
            };

        }
    }
}


