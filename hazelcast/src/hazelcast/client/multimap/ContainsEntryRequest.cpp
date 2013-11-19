//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ContainsEntryRequest.h"
#include "MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            ContainsEntryRequest::ContainsEntryRequest(const serialization::Data& key, const std::string& name, const serialization::Data& value)
            : AllPartitionsRequest(name)
            , key(&key)
            , value(&value) {

            };

            ContainsEntryRequest::ContainsEntryRequest(const serialization::Data& key, const std::string& name)
            : AllPartitionsRequest(name)
            , key(&key)
            , value(NULL) {

            };

            ContainsEntryRequest::ContainsEntryRequest(const std::string& name, const serialization::Data& value)
            : AllPartitionsRequest(name)
            , key(NULL)
            , value(&value) {

            };

            int ContainsEntryRequest::getClassId() const {
                return MultiMapPortableHook::CONTAINS_ENTRY;
            };


            void ContainsEntryRequest::writePortable(serialization::PortableWriter& writer) const {
                AllPartitionsRequest::writePortable(writer);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                util::writeNullableData(out, key);
                util::writeNullableData(out, value);
            };


        }
    }
}
