//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#include "hazelcast/client/map/ContainsKeyRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace map {
            ContainsKeyRequest::ContainsKeyRequest(const std::string& name, serialization::Data& key)
            :name(name)
            , key(key) {
            };

            int ContainsKeyRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int ContainsKeyRequest::getClassId() const {
                return PortableHook::CONTAINS_KEY;
            }


            bool ContainsKeyRequest::isRetryable() const {
                return true;
            }

            void ContainsKeyRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };

        }
    }
}
