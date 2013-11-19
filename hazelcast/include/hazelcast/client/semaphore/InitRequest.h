//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_InitRequest
#define HAZELCAST_InitRequest

#include "hazelcast/client/serialization/SerializationConstants.h"
#include "SemaphorePortableHook.h"
#include "SemaphoreRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class InitRequest : public SemaphoreRequest {
            public:
                InitRequest(const std::string& instanceName, int permitCount)
                : SemaphoreRequest(instanceName, permitCount) {

                };

                int getClassId() const {
                    return SemaphorePortableHook::INIT;
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

#endif //HAZELCAST_InitRequest
