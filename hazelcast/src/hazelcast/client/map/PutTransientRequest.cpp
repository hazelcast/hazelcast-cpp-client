//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/PutTransientRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace map {
            PutTransientRequest::PutTransientRequest(const std::string& name, const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long threadId, long ttl)
            : name(name)
            , key(key)
            , value(value)
            , threadId(threadId)
            , ttl(ttl) {

            }

            int PutTransientRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int PutTransientRequest::getClassId() const {
                return PortableHook::PUT_TRANSIENT;
            }


            void PutTransientRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", &name);
                writer.writeLong("t", threadId);
                writer.writeLong("ttl", ttl);
                writer.writeBoolean("a", false);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                out.writeData(&key);
                out.writeData(&value);
            }
        }
    }
}


