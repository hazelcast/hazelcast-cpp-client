//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/collection/ListGetRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace list {
            ListGetRequest::ListGetRequest(const std::string &name, const std::string &serviceName, int index)
            : CollectionRequest(name, serviceName)
            , index(index) {

            }

            void ListGetRequest::write(serialization::PortableWriter &writer) const {
                CollectionRequest::write(writer);
                writer.writeInt("i", index);
            }

            int ListGetRequest::getClassId() const {
                return collection::CollectionPortableHook::LIST_GET;
            }
        }
    }
}
