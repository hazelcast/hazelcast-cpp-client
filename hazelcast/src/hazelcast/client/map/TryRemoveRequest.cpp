//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/TryRemoveRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace map {
            TryRemoveRequest::TryRemoveRequest(const std::string &name, const serialization::pimpl::Data &key, long threadId, long timeout)
            :name(name)
            , key(key)
            , threadId(threadId)
            , timeout(timeout) {

            }

            int TryRemoveRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int TryRemoveRequest::getClassId() const {
                return PortableHook::TRY_REMOVE;
            }

            void TryRemoveRequest::write(serialization::PortableWriter &writer) const {
                writer.writeLong("timeout", timeout);
                writer.writeUTF("n", name);
                writer.writeLong("t", threadId);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.writeData(&key);
            }
        }
    }
}

