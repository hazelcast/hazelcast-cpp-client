//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/collection/ListRemoveRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace list {
            ListRemoveRequest::ListRemoveRequest(const std::string& name, int index)
            : CollectionRequest(name)
            , index(index) {

            }

            void ListRemoveRequest::writePortable(serialization::PortableWriter& writer) const {
                CollectionRequest::writePortable(writer);
                writer.writeInt("i", index);
            }

            int ListRemoveRequest::getClassId() const {
                return collection::CollectionPortableHook::LIST_REMOVE;
            };
        }
    }
}