//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CountDownRequest
#define HAZELCAST_CountDownRequest

#include "../serialization/SerializationConstants.h"
#include "CountDownLatchPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace countdownlatch {
            class CountDownRequest {
            public:
                CountDownRequest(const std::string& instanceName)
                : instanceName(instanceName) {

                };

                int getFactoryId() const {
                    return CountDownLatchPortableHook::F_ID;
                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getClassId() const {
                    return CountDownLatchPortableHook::COUNT_DOWN;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["name"] << instanceName;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["name"] >> instanceName;
                };
            private:

                std::string instanceName;
            };
        }
    }
}
#endif //HAZELCAST_CountDownRequest
