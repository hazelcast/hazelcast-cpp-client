//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DrainRequest
#define HAZELCAST_DrainRequest

#include "hazelcast/client/serialization/SerializationConstants.h"
#include "SemaphorePortableHook.h"
#include "SemaphoreRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class DrainRequest : public SemaphoreRequest {
            public:
                DrainRequest(const std::string& instanceName)
                : SemaphoreRequest(instanceName, -1) {

                };

                int getClassId() const {
                    return SemaphorePortableHook::DRAIN;
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


#endif //HAZELCAST_DrainRequest
