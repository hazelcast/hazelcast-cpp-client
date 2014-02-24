//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/ContainsEntryRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            ContainsEntryRequest::ContainsEntryRequest(serialization::pimpl::Data &key, const std::string &name, serialization::pimpl::Data &value)
            : AllPartitionsRequest(name)
            , hasKey(true)
            , hasValue(true)
            , key(key)
            , value(value) {

            };

            ContainsEntryRequest::ContainsEntryRequest(serialization::pimpl::Data &key, const std::string &name)
            : AllPartitionsRequest(name)
            , hasKey(true)
            , hasValue(false)
            , key(key) {

            };

            ContainsEntryRequest::ContainsEntryRequest(const std::string &name, serialization::pimpl::Data &value)
            : AllPartitionsRequest(name)
            , hasKey(false)
            , hasValue(true)
            , value(value) {

            };

            int ContainsEntryRequest::getClassId() const {
                return MultiMapPortableHook::CONTAINS_ENTRY;
            };


            void ContainsEntryRequest::write(serialization::PortableWriter &writer) const {
                AllPartitionsRequest::write(writer);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.writeBoolean(hasKey);
                if (hasKey) {
                    key.writeData(out);
                }
                out.writeBoolean(hasValue);
                if (hasValue) {
                    value.writeData(out);
                }
            };


            bool ContainsEntryRequest::isRetryable() const {
                return true;
            }
        }
    }
}
