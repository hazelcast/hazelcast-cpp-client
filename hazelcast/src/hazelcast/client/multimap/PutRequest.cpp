//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/PutRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            PutRequest::PutRequest(const std::string &name, const serialization::pimpl::Data &key, const serialization::pimpl::Data &value, int index, long threadId)
            :KeyBasedRequest(name, key)
            , value(value)
            , threadId(threadId)
            , index(index) {

            };

            int PutRequest::getClassId() const {
                return MultiMapPortableHook::PUT;
            };


            void PutRequest::write(serialization::PortableWriter &writer) const {
                writer.writeInt("i", index);
                writer.writeLong("t", threadId);
                KeyBasedRequest::write(writer);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                value.writeData(out);
            };

        }
    }
}


