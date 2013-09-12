//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ListIndexOfRequest.h"
#include "PortableWriter.h"
#include "Data.h"
#include "CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace list {
            ListIndexOfRequest::ListIndexOfRequest(const std::string& name, const serialization::Data& data, bool last)
            : CollectionRequest(name)
            , data(data)
            , last(last) {

            }

            void ListIndexOfRequest::writePortable(serialization::PortableWriter& writer) const {
                CollectionRequest::writePortable(writer);
                writer.writeBoolean("l", last);
                serialization::ObjectDataOutput & output = writer.getRawDataOutput();
                data.writeData(output);
            }

            int ListIndexOfRequest::getClassId() const {
                return collection::CollectionPortableHook::LIST_INDEX_OF;
            }
        }
    }
}