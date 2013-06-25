//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SetCount
#define HAZELCAST_SetCount

#include "../serialization/SerializationConstants.h"
#include "CountDownLatchPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace countdownlatch {
            class SetCount {
                SetCount(const std::string& instanceName, long count)
                : instanceName(instanceName)
                , count(count) {

                };

                int getFactoryId() const {
                    return CountDownLatchPortableHook::F_ID;
                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getClassId() const {
                    return CountDownLatchPortableHook::SET_COUNT;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["name"] << instanceName;
                    writer["count"] << count;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["name"] >> instanceName;
                    reader["count"] >> count;
                };
            private:

                std::string instanceName;
                long count;
            };
        }
    }
}

#endif //HAZELCAST_SetCount
