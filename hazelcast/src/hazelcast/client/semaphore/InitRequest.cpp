//
// Created by sancar koyunlu on 20/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/semaphore/InitRequest.h"
#include "hazelcast/client/semaphore/SemaphorePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace semaphore {
            InitRequest::InitRequest(const std::string& instanceName, int permitCount)
                : SemaphoreRequest(instanceName, permitCount) {

                };

                int InitRequest::getClassId() const {
                    return SemaphorePortableHook::INIT;
                };


                void InitRequest::write(serialization::PortableWriter& writer) const {
                    SemaphoreRequest::writePortable(writer);
                };

        }
    }
}
