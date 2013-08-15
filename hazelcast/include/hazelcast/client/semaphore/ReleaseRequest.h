//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ReleaseRequest
#define HAZELCAST_ReleaseRequest

#include "../serialization/SerializationConstants.h"
#include "SemaphorePortableHook.h"
#include "SemaphoreRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class ReleaseRequest : public SemaphoreRequest {
            public:
                ReleaseRequest(const std::string& instanceName, int permitCount)
                : SemaphoreRequest(instanceName, permitCount) {

                };

                int getClassId() const {
                    return SemaphorePortableHook::RELEASE;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    SemaphoreRequest::writePortable(writer);
                };


                void readPortable(serialization::PortableReader& reader) {
                    SemaphoreRequest::readPortable(reader);
                };
            };
        }
    }
}

#endif //HAZELCAST_ReleaseRequest
