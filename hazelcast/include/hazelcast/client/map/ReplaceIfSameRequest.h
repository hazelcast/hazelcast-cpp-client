//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_REPLACE_IF_SAME_REQUEST
#define HAZELCAST_MAP_REPLACE_IF_SAME_REQUEST

#include "../serialization/Data.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class ReplaceIfSameRequest {
            public:
                ReplaceIfSameRequest(const std::string& name, serialization::Data& key, serialization::Data& testValue, serialization::Data& value, int threadId)
                :name(name)
                , key(key)
                , value(value)
                , testValue(testValue)
                , threadId(threadId) {

                };

                int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::REPLACE_IF_SAME;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeInt("t", threadId);
                    serialization::BufferedDataOutput *out = writer.getRawDataOutput();
                    key.writeData(*out);
                    value.writeData(*out);
                    testValue.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("n");
                    threadId = reader.readInt("t");
                    serialization::BufferedDataInput *in = reader.getRawDataInput();
                    key.readData(*in);
                    value.readData(*in);
                    testValue.readData(*in);
                };

            private:
                serialization::Data& key;
                serialization::Data& value;
                serialization::Data& testValue;
                std::string name;
                int threadId;
            };
        }
    }
}

#endif //HAZELCAST_MAP_REPLACE_IF_SAME_REQUEST
