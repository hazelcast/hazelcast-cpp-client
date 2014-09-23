//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/RemoveIfSameRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace map {
            RemoveIfSameRequest::RemoveIfSameRequest(const std::string &name, serialization::pimpl::Data &key, serialization::pimpl::Data &value, long threadId)
            :name(name)
            , key(key)
            , value(value)
            , threadId(threadId) {

            }

            int RemoveIfSameRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int RemoveIfSameRequest::getClassId() const {
                return PortableHook::REMOVE_IF_SAME;
            }

            void RemoveIfSameRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("t", threadId);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                key.writeData(out);
                value.writeData(out);
            }

        }
    }
}


