//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#include "hazelcast/client/map/ContainsKeyRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace map {
            ContainsKeyRequest::ContainsKeyRequest(const std::string &name, const serialization::pimpl::Data &key, long threadId)
            :name(name)
            , key(key)
            , threadId(threadId){
            }

            int ContainsKeyRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int ContainsKeyRequest::getClassId() const {
                return PortableHook::CONTAINS_KEY;
            }


            bool ContainsKeyRequest::isRetryable() const {
                return true;
            }

            void ContainsKeyRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("threadId", threadId);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.writeData(&key);
            }

        }
    }
}

