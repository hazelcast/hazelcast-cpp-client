//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_TRY_REMOVE_REQUEST
#define HAZELCAST_MAP_TRY_REMOVE_REQUEST

#include "../serialization/Data.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class TryRemoveRequest : public Portable{
            public:
                TryRemoveRequest(const std::string& name, serialization::Data& key, int threadId, long timeout)
                :name(name)
                , key(key)
                , threadId(threadId)
                , timeout(timeout) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                };

                int getClassId() const {
                    return PortableHook::TRY_REMOVE;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeLong("timeout", timeout);
                    writer.writeUTF("n", name);
                    writer.writeInt("t", threadId);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    key.writeData(out);
                };


                void readPortable(serialization::PortableReader& reader) {
                    timeout = reader.readLong("timeout");
                    name = reader.readUTF("n");
                    threadId = reader.readInt("t");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    key.readData(in);
                };

            private:
                serialization::Data& key;
                std::string name;
                int threadId;
                int timeout;
            };
        }
    }
}

#endif //HAZELCAST_MAP_REMOVE_REQUEST
