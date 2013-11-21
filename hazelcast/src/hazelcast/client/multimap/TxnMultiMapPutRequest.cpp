//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/TxnMultiMapPutRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            TxnMultiMapPutRequest::TxnMultiMapPutRequest(const std::string& name, const serialization::Data& key, const serialization::Data& value)
            : TxnMultiMapRequest(name)
            , key(key)
            , value(value) {

            }

            int TxnMultiMapPutRequest::getClassId() const {
                return MultiMapPortableHook::TXN_MM_PUT;
            }

            void TxnMultiMapPutRequest::writePortable(serialization::PortableWriter& writer) const {
                TxnMultiMapRequest::writePortable(writer);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
                value.writeData(out);
            };

        }
    }
}