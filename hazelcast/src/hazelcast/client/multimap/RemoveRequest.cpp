//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/RemoveRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            RemoveRequest::RemoveRequest(const std::string& name, const serialization::Data& key, const serialization::Data& value, int threadId)
            : KeyBasedRequest(name, key)
            , value(value)
            , threadId(threadId) {

            };

            int RemoveRequest::getClassId() const {
                return MultiMapPortableHook::REMOVE;
            };


            void RemoveRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeInt("t", threadId);
                KeyBasedRequest::writePortable(writer);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                value.writeData(out);
            };

        }
    }
}

