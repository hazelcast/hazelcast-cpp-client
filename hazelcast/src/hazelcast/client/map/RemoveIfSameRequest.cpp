//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "RemoveIfSameRequest.h"
#include "PortableHook.h"
#include "PortableReader.h"
#include "PortableWriter.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace map {
            RemoveIfSameRequest::RemoveIfSameRequest(const std::string& name, serialization::Data& key, serialization::Data& value, int threadId)
            :name(name)
            , key(key)
            , value(value)
            , threadId(threadId) {

            };

            int RemoveIfSameRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int RemoveIfSameRequest::getClassId() const {
                return PortableHook::REMOVE_IF_SAME;
            }

            void RemoveIfSameRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeInt("t", threadId);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
                value.writeData(out);
            };

            void RemoveIfSameRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                threadId = reader.readInt("t");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key.readData(in);
                value.readData(in);
            };

        }
    }
}

