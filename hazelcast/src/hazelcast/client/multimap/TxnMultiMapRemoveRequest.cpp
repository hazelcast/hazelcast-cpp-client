//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/TxnMultiMapRemoveRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            TxnMultiMapRemoveRequest::TxnMultiMapRemoveRequest(const std::string& name, const serialization::pimpl::Data& key)
            : TxnMultiMapRequest(name)
            , key(key)
            , hasValue(false) {

            }

            TxnMultiMapRemoveRequest::TxnMultiMapRemoveRequest(const std::string& name, const serialization::pimpl::Data& key, const serialization::pimpl::Data& value)
            : TxnMultiMapRequest(name)
            , key(key)
            , hasValue(true)
            , value(value) {

            }

            int TxnMultiMapRemoveRequest::getClassId() const {
                return MultiMapPortableHook::TXN_MM_REMOVE;
            }

            void TxnMultiMapRemoveRequest::write(serialization::PortableWriter& writer) const {
                TxnMultiMapRequest::write(writer);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
                out.writeBoolean(hasValue);
                if (hasValue)
                    value.writeData(out);
            }

        }
    }
}
