//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_DELETE_REQUEST
#define HAZELCAST_MAP_DELETE_REQUEST

#include "../serialization/Data.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class DeleteRequest : public Portable {
            public:
                DeleteRequest(const std::string& name, serialization::Data& key, int threadId)
                :name(name)
                , key(key)
                , threadId(threadId) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::DELETE;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("name", name);
                    writer.writeInt("t", threadId);
                    serialization::BufferedDataOutput *out = writer.getRawDataOutput();
                    key.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("name");
                    threadId = reader.readInt("t");
                    serialization::BufferedDataInput *in = reader.getRawDataInput();
                    key.readData(*in);
                };
            private:
                serialization::Data& key;
                std::string name;
                int threadId;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
