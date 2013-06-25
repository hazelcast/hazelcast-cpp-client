//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_InitRequest
#define HAZELCAST_InitRequest

#include "../serialization/SerializationConstants.h"
#include "SemaphorePortableHook.h"
#include "SemaphoreRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class InitRequest : public SemaphoreRequest {
                InitRequest(const std::string& instanceName, int permitCount)
                : SemaphoreRequest(instanceName, permitCount) {

                };

                int getClassId() const {
                    return SemaphorePortableHook::INIT;
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

#endif //HAZELCAST_InitRequest
