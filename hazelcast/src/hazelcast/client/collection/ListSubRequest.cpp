//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ListSubRequest.h"
#include "CollectionPortableHook.h"
#include "PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace list {
            ListSubRequest::ListSubRequest(const std::string& name, int from, int to)
            : CollectionRequest(name)
            , from(from)
            , to(to) {

            }

            int ListSubRequest::getClassId() const {
                return collection::CollectionPortableHook::LIST_SUB;
            }

            void ListSubRequest::writePortable(serialization::PortableWriter& writer) const {
                collection::CollectionRequest::writePortable(writer);
                writer.writeInt("f", from);
                writer.writeInt("t", to);
            }
        }
    }
}