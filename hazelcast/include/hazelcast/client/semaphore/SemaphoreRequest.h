//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SemaphoreRequest
#define HAZELCAST_SemaphoreRequest

#include "../serialization/SerializationConstants.h"
#include "SemaphorePortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class SemaphoreRequest {
            public:
                SemaphoreRequest(const std::string& instanceName, int permitCount)
                :instanceName(instanceName)
                , permitCount(permitCount) {

                };

                virtual int getFactoryId() const {
                    return SemaphorePortableHook::F_ID;
                };

                virtual int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << instanceName;
                    writer["p"] << permitCount;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> instanceName;
                    reader["p"] >> permitCount;
                };
            private:

                std::string instanceName;
                int permitCount;
            };
        }
    }
}

#endif //HAZELCAST_SemaphoreRequest
