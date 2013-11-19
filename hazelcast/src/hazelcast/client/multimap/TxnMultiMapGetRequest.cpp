//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnMultiMapGetRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            TxnMultiMapGetRequest::TxnMultiMapGetRequest(const std::string& name, const serialization::Data& data)
            : TxnMultiMapRequest(name)
            , data(data) {

            }

            int TxnMultiMapGetRequest::getClassId() const {
                return MultiMapPortableHook::TXN_MM_GET;
            }

            void TxnMultiMapGetRequest::writePortable(serialization::PortableWriter& writer) const {
                TxnMultiMapRequest::writePortable(writer);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                data.writeData(out);
            };
        }
    }
}
