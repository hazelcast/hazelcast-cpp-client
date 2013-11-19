//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/collection/ListGetRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace list {
            ListGetRequest::ListGetRequest(const std::string& name, int index)
            : CollectionRequest(name)
            , index(index) {

            }

            void ListGetRequest::writePortable(serialization::PortableWriter& writer) const {
                CollectionRequest::writePortable(writer);
                writer.writeInt("i", index);
            }

            int ListGetRequest::getClassId() const {
                return collection::CollectionPortableHook::LIST_GET;
            };
        }
    }
}