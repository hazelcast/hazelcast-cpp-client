//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/GetRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/map/PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            GetRequest::GetRequest(const std::string& name, const serialization::pimpl::Data& key, long threadId)
            : name(name)
            , async(false) //MTODO implement getAsync request on IMAP
            , key(key)
            , threadId(threadId) {

            }

            int GetRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int GetRequest::getClassId() const {
                return PortableHook::GET;
            }

            bool GetRequest::isRetryable() const {
                return true;
            }

            void GetRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeBoolean("a", async);
                writer.writeLong("threadId", threadId);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            }
        }
    }
}

