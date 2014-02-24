//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/collection/ListSetRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace list {
            ListSetRequest::ListSetRequest(const std::string &name, const std::string &serviceName, const serialization::pimpl::Data &data, int index)
            : CollectionRequest(name, serviceName)
            , data(data)
            , index(index) {

            }

            void ListSetRequest::write(serialization::PortableWriter &writer) const {
                CollectionRequest::write(writer);
                writer.writeInt("i", index);
                serialization::ObjectDataOutput &output = writer.getRawDataOutput();
                data.writeData(output);
            }

            int ListSetRequest::getClassId() const {
                return collection::CollectionPortableHook::LIST_SET;
            }
        }
    }
}