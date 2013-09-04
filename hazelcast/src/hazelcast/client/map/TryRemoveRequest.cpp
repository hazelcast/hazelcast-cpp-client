//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TryRemoveRequest.h"
#include "Data.h"
#include "PortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace map {
            TryRemoveRequest::TryRemoveRequest(const std::string& name, serialization::Data& key, int threadId, long timeout)
            :name(name)
            , key(key)
            , threadId(threadId)
            , timeout(timeout) {

            };

            int TryRemoveRequest::getFactoryId() const {
                return PortableHook::F_ID;
            };

            int TryRemoveRequest::getClassId() const {
                return PortableHook::TRY_REMOVE;
            };

            void TryRemoveRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeLong("timeout", timeout);
                writer.writeUTF("n", name);
                writer.writeInt("t", threadId);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };

            void TryRemoveRequest::readPortable(serialization::PortableReader& reader) {
                timeout = reader.readLong("timeout");
                name = reader.readUTF("n");
                threadId = reader.readInt("t");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key.readData(in);
            };
        }
    }
}
