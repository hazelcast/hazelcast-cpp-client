//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/LockRequest.h"
#include "Data.h"
#include "PortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace map {
            LockRequest::LockRequest(const std::string& name, serialization::Data& key, int threadId, long ttl, long timeout)
            :name(name)
            , key(key)
            , threadId(threadId)
            , ttl(ttl)
            , timeout(timeout) {
            };

            LockRequest::LockRequest(const std::string& name, serialization::Data& key, int threadId)
            :name(name)
            , key(key)
            , threadId(threadId)
            , ttl(-1)
            , timeout(-1) {
            };

            int LockRequest::getFactoryId() const {
                return PortableHook::F_ID;
            };

            int LockRequest::getClassId() const {
                return PortableHook::LOCK;
            };

            void LockRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeInt("tid", threadId);
                writer.writeLong("ttl", ttl);
                writer.writeLong("timeout", timeout);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };

            void LockRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                threadId = reader.readInt("tid");
                ttl = reader.readLong("ttl");
                timeout = reader.readLong("timeout");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key.readData(in);
            };
        }
    }
}

