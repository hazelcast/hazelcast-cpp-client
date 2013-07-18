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

                int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getClassId() const {
                    return TopicPortableHook::PUBLISH;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", instanceName);
                    serialization::BufferedDataOutput *out = writer.getRawDataOutput();
                    message.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    instanceName = reader.readUTF("n");
                    serialization::BufferedDataInput *in = reader.getRawDataInput();
                    message.readData(*in);
                };
            private:
                serialization::Data message;
                std::string instanceName;
            };
        }
    }
}


#endif //HAZELCAST_PublishRequest
