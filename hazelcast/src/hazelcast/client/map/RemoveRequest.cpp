//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/RemoveRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace map {
            RemoveRequest::RemoveRequest(const std::string &name, serialization::pimpl::Data &key, long threadId)
            :name(name)
            , async(false) //MTODO implement removeAsync request on IMAP
            , key(key)
            , threadId(threadId) {

            }

            int RemoveRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int RemoveRequest::getClassId() const {
                return PortableHook::REMOVE;
            }

            void RemoveRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("t", threadId);
                writer.writeBoolean("a", async);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                key.writeData(out);
            }
        }
    }
}
