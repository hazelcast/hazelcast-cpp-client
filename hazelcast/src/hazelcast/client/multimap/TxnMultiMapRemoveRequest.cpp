//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/TxnMultiMapRemoveRequest.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            TxnMultiMapRemoveRequest::TxnMultiMapRemoveRequest(const std::string& name, serialization::Data& key)
            : TxnMultiMapRequest(name)
            , key(key)
            , value(NULL) {

            }

            TxnMultiMapRemoveRequest::TxnMultiMapRemoveRequest(const std::string& name, serialization::Data& key, const serialization::Data& value)
            : TxnMultiMapRequest(name)
            , key(key)
            , value(&value) {

            }

            int TxnMultiMapRemoveRequest::getClassId() const {
                return MultiMapPortableHook::TXN_MM_REMOVE;
            }

            void TxnMultiMapRemoveRequest::writePortable(serialization::PortableWriter& writer) const {
                TxnMultiMapRequest::writePortable(writer);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
                util::writeNullableData(out, value);
            };

        }
    }
}