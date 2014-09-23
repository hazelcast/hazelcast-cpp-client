//
// Created by sancar koyunlu on 20/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/semaphore/AvailableRequest.h"
#include "hazelcast/client/semaphore/SemaphorePortableHook.h"

namespace hazelcast {
    namespace client {
        namespace semaphore {
            AvailableRequest::AvailableRequest(const std::string &instanceName)
            : SemaphoreRequest(instanceName, -1) {

            }

            int AvailableRequest::getClassId() const {
                return SemaphorePortableHook::AVAILABLE;
            }

            bool AvailableRequest::isRetryable() const {
                return true;
            }
        }
    }
}

