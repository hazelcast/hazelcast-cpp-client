//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PublishRequest
#define HAZELCAST_PublishRequest


#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "TopicPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace topic {
            class PublishRequest {
            public:
                PublishRequest(const std::string& instanceName, const serialization::Data& message)
                : instanceName(instanceName)
                , message(message) {

                };

                int getFactoryId() const {
                    return TopicPortableHook::F_ID;
                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getClassId() const {
                    return TopicPortableHook::PUBLISH;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << instanceName;
                    writer << message;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> instanceName;
                    reader >> message;
                };
            private:
                serialization::Data message;
                std::string instanceName;
            };
        }
    }
}


#endif //HAZELCAST_PublishRequest
