//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "CollectionRemoveRequest.h"
#include "PortableWriter.h"
#include "Data.h"
#include "CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            CollectionRemoveRequest::CollectionRemoveRequest(const std::string& name, const serialization::Data& data)
            : CollectionRequest(name)
            , data(data) {

            }

            int CollectionRemoveRequest::getClassId() const {
                return CollectionPortableHook::COLLECTION_REMOVE;
            }

            void CollectionRemoveRequest::writePortable(serialization::PortableWriter& writer) const {
                CollectionRequest::writePortable(writer);
                serialization::ObjectDataOutput & output = writer.getRawDataOutput();
                data.writeData(output);
            }
        }
    }
}
