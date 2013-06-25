//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DestroyRequest
#define HAZELCAST_DestroyRequest

#include "../serialization/SerializationConstants.h"
#include "AtomicLongPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            class DestroyRequest {
                DestroyRequest(const std::string& instanceName)
                :instanceName(instanceName) {

                };

                int getFactoryId() const {
                    return AtomicLongPortableHook::F_ID;
                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getClassId() const {
                    return AtomicLongPortableHook::DESTROY;
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
