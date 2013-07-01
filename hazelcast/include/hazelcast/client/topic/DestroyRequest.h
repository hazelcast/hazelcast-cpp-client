//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_Topic_DestroyRequest
#define HAZELCAST_Topic_DestroyRequest

#include "../serialization/SerializationConstants.h"
#include "TopicPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace topic {
            class DestroyRequest {
            public:
                DestroyRequest(const std::string& instanceName)
                : instanceName(instanceName) {

                };

                int getFactoryId() const {
                    return TopicPortableHook::F_ID;
                };

                int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getClassId() const {
                    return TopicPortableHook::DESTROY;
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
