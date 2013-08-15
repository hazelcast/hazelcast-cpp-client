//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AcquireRequest
#define HAZELCAST_AcquireRequest

#include "../serialization/SerializationConstants.h"
#include "SemaphorePortableHook.h"
#include "SemaphoreRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class AcquireRequest : public SemaphoreRequest {
            public:
                AcquireRequest(const std::string& instanceName, int permitCount, long timeout)
                : SemaphoreRequest(instanceName, permitCount)
                , timeout(timeout) {

                };

                int getClassId() const {
                    return SemaphorePortableHook::ACQUIRE;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    SemaphoreRequest::writePortable(writer);
                    writer.writeLong("t",timeout);
                };


                void readPortable(serialization::PortableReader& reader) {
                    SemaphoreRequest::readPortable(reader);
                    timeout = reader.readLong("t");
                };
            private:

                long timeout;
            };
        }
    }
}

#endif //HAZELCAST_AcquireRequest
