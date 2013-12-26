//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/TxnMultiMapRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            TxnMultiMapRequest::TxnMultiMapRequest(const std::string& name)
            :name(name) {

            }

            int TxnMultiMapRequest::getFactoryId() const {
                return MultiMapPortableHook::F_ID;
            }

            void TxnMultiMapRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
            };
        }
    }
}