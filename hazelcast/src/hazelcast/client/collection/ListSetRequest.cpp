//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ListSetRequest.h"
#include "PortableWriter.h"
#include "CollectionPortableHook.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace list {
            ListSetRequest::ListSetRequest(const std::string& name, const serialization::Data& data, bool last)
            : CollectionRequest(name)
            , data(data)
            , last(last) {

            }

            void ListSetRequest::writePortable(serialization::PortableWriter& writer) const {
                CollectionRequest::writePortable(writer);
                writer.writeBoolean("l", last);
                serialization::ObjectDataOutput & output = writer.getRawDataOutput();
                data.writeData(output);
            }

            int ListSetRequest::getClassId() const {
                return collection::CollectionPortableHook::LIST_SET;
            }
        }
    }
}