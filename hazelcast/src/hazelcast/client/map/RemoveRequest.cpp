//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/RemoveRequest.h"
#include "Data.h"
#include "PortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace map {
            RemoveRequest::RemoveRequest(const std::string& name, serialization::Data& key, int threadId)
            :name(name)
            , key(key)
            , threadId(threadId) {

            };

            int RemoveRequest::getFactoryId() const {
                return PortableHook::F_ID;
            };

            int RemoveRequest::getClassId() const {
                return PortableHook::REMOVE;
            };

            void RemoveRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeInt("t", threadId);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };

            void RemoveRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                threadId = reader.readInt("t");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key.readData(in);
            };
        }
    }
}