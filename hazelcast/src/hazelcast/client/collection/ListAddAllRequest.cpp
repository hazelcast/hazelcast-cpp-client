//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/collection/ListAddAllRequest.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace list {
            ListAddAllRequest::ListAddAllRequest(const std::string& name, const std::vector<serialization::Data>& valueList, int index)
            : CollectionAddAllRequest(name, valueList)
            , index(index) {

            }

            int ListAddAllRequest::getClassId() const {
                return collection::CollectionPortableHook::LIST_ADD_ALL;
            }

            void ListAddAllRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeInt("i", index);
                CollectionAddAllRequest::writePortable(writer);
            }
        }
    }
}