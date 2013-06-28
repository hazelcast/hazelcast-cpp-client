//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AvailableRequest
#define HAZELCAST_AvailableRequest

#include "../serialization/SerializationConstants.h"
#include "SemaphorePortableHook.h"
#include "SemaphoreRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class AvailableRequest : public SemaphoreRequest {
            public:
                AvailableRequest(const std::string& instanceName)
                : SemaphoreRequest(instanceName, -1) {

                };

                int getClassId() const {
                    return SemaphorePortableHook::AVAILABLE;
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

#endif //HAZELCAST_AvailableRequest
