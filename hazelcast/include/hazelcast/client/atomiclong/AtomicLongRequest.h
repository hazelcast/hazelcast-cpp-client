//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AtomicLongRequest
#define HAZELCAST_AtomicLongRequest

#include "../serialization/SerializationConstants.h"
#include "AtomicLongPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            class AtomicLongRequest {
            public:
                AtomicLongRequest(const std::string& instanceName, long delta)
                :instanceName(instanceName)
                , delta(delta) {

                };

                virtual int getFactoryId() const {
                    return AtomicLongPortableHook::F_ID;
                };

                virtual int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << instanceName;
                    writer["d"] << delta;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> instanceName;
                    reader["d"] >> delta;
                };
            private:
                long delta;
                std::string instanceName;
            };
        }
    }
}

#endif //HAZELCAST_AtomicLongRequest
