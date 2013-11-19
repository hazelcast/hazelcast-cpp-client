//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "PutTransientRequest.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/map/PortableHook.h"

#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace map {
            PutTransientRequest::PutTransientRequest(const std::string& name, serialization::Data& key, serialization::Data& value, int threadId, long ttl)
            :name(name)
            , key(key)
            , value(value)
            , threadId(threadId)
            , ttl(ttl) {

            };

            int PutTransientRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int PutTransientRequest::getClassId() const {
                return PortableHook::PUT_TRANSIENT;
            }


            void PutTransientRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeInt("thread", threadId);
                writer.writeLong("ttl", ttl);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };
        }
    }
}

