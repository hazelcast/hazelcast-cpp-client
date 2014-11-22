//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/TxnMultiMapGetRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            TxnMultiMapGetRequest::TxnMultiMapGetRequest(const std::string &name, const serialization::pimpl::Data &data)
            : TxnMultiMapRequest(name)
            , data(data) {

            }

            int TxnMultiMapGetRequest::getClassId() const {
                return MultiMapPortableHook::TXN_MM_GET;
            }

            void TxnMultiMapGetRequest::write(serialization::PortableWriter &writer) const {
                TxnMultiMapRequest::write(writer);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.writeData(&data);
            }
        }
    }
}

