//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/collection/CollectionContainsRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            CollectionContainsRequest::CollectionContainsRequest(const std::string &name, const std::string &serviceName, const std::vector<serialization::pimpl::Data> &valueSet)
            : CollectionRequest(name, serviceName)
            , valueSet(valueSet) {

            }

            void CollectionContainsRequest::write(serialization::PortableWriter &writer) const {
                CollectionRequest::write(writer);
                serialization::ObjectDataOutput &output = writer.getRawDataOutput();
                int size = valueSet.size();
                output.writeInt(size);
                for (int i = 0; i < size; i++) {
                    valueSet[i].writeData(output);
                }
            }

            int CollectionContainsRequest::getClassId() const {
                return CollectionPortableHook::COLLECTION_CONTAINS;
            }


        }
    }
}

