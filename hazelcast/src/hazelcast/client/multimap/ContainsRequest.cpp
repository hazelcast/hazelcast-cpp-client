//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/ContainsRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            ContainsRequest::ContainsRequest(const std::string &name, serialization::pimpl::Data &value)
            : AllPartitionsRequest(name)
            , value(value) {

            }

            int ContainsRequest::getClassId() const {
                return MultiMapPortableHook::CONTAINS_ENTRY;
            }


            void ContainsRequest::write(serialization::PortableWriter &writer) const {
                AllPartitionsRequest::write(writer);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.writeBoolean(true);
                value.writeData(out);
            }

            bool ContainsRequest::isRetryable() const {
                return true;
            }
        }
    }
}

