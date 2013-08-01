//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_REMOVE_REQUEST
#define HAZELCAST_MAP_REMOVE_REQUEST

#include "../serialization/Data.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class RemoveRequest : public Portable{
            public:
                RemoveRequest(const std::string& name, serialization::Data& key, int threadId)
                :name(name)
                , key(key)
                , threadId(threadId) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                };

                int getClassId() const {
                    return PortableHook::REMOVE;
                };


                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeInt("t", threadId);
                    serialization::ObjectDataOutput *out = writer.getRawDataOutput();
                    key.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("n");
                    threadId = reader.readInt("t");
                    serialization::ObjectDataInput *in = reader.getRawDataInput();
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

#endif //HAZELCAST_MAP_REMOVE_REQUEST
