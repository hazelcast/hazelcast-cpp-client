//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ListGetRequest.h"
#include "PortableWriter.h"
#include "CollectionPortableHook.h"

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