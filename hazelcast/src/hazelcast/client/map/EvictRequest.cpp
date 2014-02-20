//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/EvictRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace map {
            EvictRequest::EvictRequest(const std::string &name, serialization::Data &key, long threadId)
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

            void EvictRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("t", threadId);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                key.writeData(out);
            };

        }
    }
}
