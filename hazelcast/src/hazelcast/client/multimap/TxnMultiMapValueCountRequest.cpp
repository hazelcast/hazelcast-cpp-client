//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/TxnMultiMapValueCountRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            TxnMultiMapValueCountRequest::TxnMultiMapValueCountRequest(const std::string &name, const serialization::Data &data)
            : TxnMultiMapRequest(name)
            , data(data) {

            }

            int TxnMultiMapValueCountRequest::getClassId() const {
                return MultiMapPortableHook::TXN_MM_VALUE_COUNT;
            }

            void TxnMultiMapValueCountRequest::write(serialization::PortableWriter &writer) const {
                TxnMultiMapRequest::write(writer);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                data.writeData(out);
            };

        }
    }
}