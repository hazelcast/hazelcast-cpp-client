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
                ReleaseRequest(const std::string& instanceName, int permitCount)
                : SemaphoreRequest(instanceName, permitCount) {

                };

                int getClassId() const {
                    return SemaphorePortableHook::RELEASE;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    SemaphoreRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    SemaphoreRequest::readPortable(reader);
                };
            };
        }
    }
}

#endif //HAZELCAST_ReleaseRequest
