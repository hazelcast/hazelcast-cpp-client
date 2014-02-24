//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/collection/CollectionAddRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            CollectionAddRequest::CollectionAddRequest(const std::string &name, const std::string &serviceName, const serialization::pimpl::Data &data)
            : CollectionRequest(name, serviceName)
            , data(data) {

            }

            int CollectionAddRequest::getClassId() const {
                return CollectionPortableHook::COLLECTION_ADD;
            }

            void CollectionAddRequest::write(serialization::PortableWriter &writer) const {
                CollectionRequest::write(writer);
                serialization::ObjectDataOutput &output = writer.getRawDataOutput();
                data.writeData(output);
            }
        }
    }
}
