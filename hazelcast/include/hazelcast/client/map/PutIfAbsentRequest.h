//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_PUT_IF_ABSENT_REQUEST
#define HAZELCAST_MAP_PUT_IF_ABSENT_REQUEST

#include "../serialization/Data.h"
#include "PortableHook.h"
#include "Portable.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class PutIfAbsentRequest : public Portable{
            public:
                PutIfAbsentRequest(const std::string& name, serialization::Data& key, serialization::Data& value, int threadId, long ttl)
                :name(name)
                , key(key)
                , value(value)
                , threadId(threadId)
                , ttl(ttl) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::PUT_IF_ABSENT;
                }


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeInt("thread", threadId);
                    writer.writeLong("ttl", ttl);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    key.writeData(out);
                };


                void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("n");
                    threadId = reader.readInt("thread");
                    ttl = reader.readLong("ttl");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    key.readData(in);
                };

            private:
                serialization::Data& key;
                serialization::Data& value;
                std::string name;
                int threadId;
                long ttl;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
