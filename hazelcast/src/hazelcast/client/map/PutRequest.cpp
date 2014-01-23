//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/PutRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/map/PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            PutRequest::PutRequest(const std::string &name, serialization::Data &key, serialization::Data &value, long threadId, long ttl)
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


            void PutRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("t", threadId);
                writer.writeLong("ttl", ttl);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                key.writeData(out);
                value.writeData(out);
            };

        }
    }
}