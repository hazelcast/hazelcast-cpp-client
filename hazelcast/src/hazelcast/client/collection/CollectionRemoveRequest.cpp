//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/collection/CollectionRemoveRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            CollectionRemoveRequest::CollectionRemoveRequest(const std::string &name, const std::string &serviceName, const serialization::pimpl::Data &data)
            : CollectionRequest(name, serviceName)
            , data(data) {

            }

            int CollectionRemoveRequest::getClassId() const {
                return CollectionPortableHook::COLLECTION_REMOVE;
            }

            void CollectionRemoveRequest::write(serialization::PortableWriter &writer) const {
                CollectionRequest::write(writer);
                serialization::ObjectDataOutput &output = writer.getRawDataOutput();
                output.writeData(&data);
            }
        }
    }
}

