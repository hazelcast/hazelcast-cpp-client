//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/collection/ListAddRequest.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace list {
            ListAddRequest::ListAddRequest(const std::string &name, const std::string &serviceName, const serialization::pimpl::Data &valueList, int index)
            : collection::CollectionAddRequest(name, serviceName, valueList)
            , index(index) {

            }

            int ListAddRequest::getClassId() const {
                return collection::CollectionPortableHook::LIST_ADD;
            }

            void ListAddRequest::write(serialization::PortableWriter &writer) const {
                writer.writeInt("i", index);
                collection::CollectionAddRequest::write(writer);
            }
        }
    }
}

