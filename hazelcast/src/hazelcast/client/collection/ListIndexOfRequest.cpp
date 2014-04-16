//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/collection/ListIndexOfRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace list {
            ListIndexOfRequest::ListIndexOfRequest(const std::string &name, const std::string &serviceName, const serialization::pimpl::Data &data, bool last)
            : CollectionRequest(name, serviceName)
            , data(data)
            , last(last) {

            }

            void ListIndexOfRequest::write(serialization::PortableWriter &writer) const {
                CollectionRequest::write(writer);
                writer.writeBoolean("l", last);
                serialization::ObjectDataOutput &output = writer.getRawDataOutput();
                data.writeData(output);
            }

            int ListIndexOfRequest::getClassId() const {
                return collection::CollectionPortableHook::LIST_INDEX_OF;
            }
        }
    }
}
