//
// Created by sancar koyunlu on 20/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/semaphore/SemaphoreRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/semaphore/SemaphorePortableHook.h"

namespace hazelcast {
    namespace client {
        namespace semaphore {
            SemaphoreRequest::SemaphoreRequest(const std::string& instanceName, int permitCount)
                :instanceName(instanceName)
                , permitCount(permitCount) {

                };

                int SemaphoreRequest::getFactoryId() const {
                    return SemaphorePortableHook::F_ID;
                };


                void SemaphoreRequest::write(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", instanceName);
                    writer.writeInt("p", permitCount);
                };

        }
    }
}
