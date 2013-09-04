//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "EvictRequest.h"
#include "PortableHook.h"
#include "Data.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace map {
            EvictRequest::EvictRequest(const std::string& name, serialization::Data& key, int threadId)
            :name(name)
            , key(key)
            , threadId(threadId) {

            };

            int EvictRequest::getFactoryId() const {
                return PortableHook::F_ID;
            };

            int EvictRequest::getClassId() const {
                return PortableHook::EVICT;
            };

            void EvictRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("name", name);
                writer.writeInt("t", threadId);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };

            void EvictRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("name");
                threadId = reader.readInt("t");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key.readData(in);
            };
        }
    }
}
