//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnMultiMapValueCountRequest.h"
#include "MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            TxnMultiMapValueCountRequest::TxnMultiMapValueCountRequest(const std::string& name, const serialization::Data& data)
            : TxnMultiMapRequest(name)
            , data(data) {

            }

            int TxnMultiMapValueCountRequest::getClassId() const {
                return MultiMapPortableHook::TXN_MM_VALUE_COUNT;
            }

            void TxnMultiMapValueCountRequest::writePortable(serialization::PortableWriter& writer) const {
                TxnMultiMapRequest::writePortable(writer);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                data.writeData(out);
            };

        }
    }
}