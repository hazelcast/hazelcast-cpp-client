//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/PutRequest.h"
#include "PortableWriter.h"
#include "PortableReader.h"
#include "PortableHook.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace map {
            PutRequest::PutRequest(const std::string& name, serialization::Data& key, serialization::Data& value, int threadId, long ttl)
            :name(name)
            , key(key)
            , value(value)
            , threadId(threadId)
            , ttl(ttl) {

            };

            int PutRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int PutRequest::getClassId() const {
                return PortableHook::PUT;
            }


            void PutRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeInt("t", threadId);
                writer.writeLong("ttl", ttl);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
                value.writeData(out);
            };


            void PutRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                threadId = reader.readInt("t");
                ttl = reader.readLong("ttl");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key.readData(in);
                value.readData(in);
            };

        }
    }
}