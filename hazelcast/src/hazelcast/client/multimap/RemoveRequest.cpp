//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/RemoveRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            RemoveRequest::RemoveRequest(const std::string &name, const serialization::pimpl::Data &key, const serialization::pimpl::Data &value, long threadId)
            : KeyBasedRequest(name, key)
            , value(value)
            , threadId(threadId) {

            }

            int RemoveRequest::getClassId() const {
                return MultiMapPortableHook::REMOVE;
            }


            void RemoveRequest::write(serialization::PortableWriter &writer) const {
                writer.writeLong("t", threadId);
                KeyBasedRequest::write(writer);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.writeData(&value);
            }

        }
    }
}


