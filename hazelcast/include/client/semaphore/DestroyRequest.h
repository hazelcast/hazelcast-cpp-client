//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DestroyRequest
#define HAZELCAST_DestroyRequest


#include "../serialization/SerializationConstants.h"
#include "SemaphorePortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class DestroyRequest {
            public:
                DestroyRequest(const std::string& instanceName)
                :instanceName(instanceName) {
                };

                int getClassId() const {
                    return SemaphorePortableHook::DESTROY;
                };

                int getFactoryId() const {
                    return SemaphorePortableHook::F_ID;
                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << instanceName;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> instanceName;
                };
            private:

                std::string instanceName;
            };
        }
    }
}


#endif //HAZELCAST_DestroyRequest
