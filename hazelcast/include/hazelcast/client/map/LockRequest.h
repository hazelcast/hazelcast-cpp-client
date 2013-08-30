//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_LOCK_REQUEST
#define HAZELCAST_LOCK_REQUEST

#include "../serialization/Data.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class LockRequest : public Portable {
            public:
                LockRequest(const std::string& name, serialization::Data& key, int threadId, long ttl, long timeout)
                :name(name)
                , key(key)
                , threadId(threadId)
                , ttl(ttl)
                , timeout(timeout) {
                };

                LockRequest(const std::string& name, serialization::Data& key, int threadId)
                :name(name)
                , key(key)
                , threadId(threadId)
                , ttl(-1)
                , timeout(-1) {
                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                };

                int getClassId() const {
                    return PortableHook::LOCK;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeInt("tid", threadId);
                    writer.writeLong("ttl", ttl);
                    writer.writeLong("timeout", timeout);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    key.writeData(out);
                };


                void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("n");
                    threadId = reader.readInt("tid");
                    ttl = reader.readLong("ttl");
                    timeout = reader.readLong("timeout");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    key.readData(in);
                };
            private:
                serialization::Data& key;
                std::string name;
                int threadId;
                long ttl;
                long timeout;
            };
        }
    }
}

#endif //HAZELCAST_LOCK_REQUEST
