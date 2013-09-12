//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "CollectionCompareAndRemoveRequest.h"
#include "PortableWriter.h"
#include "Data.h"
#include "CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            CollectionCompareAndRemoveRequest::CollectionCompareAndRemoveRequest(const std::string& name, const std::vector<serialization::Data>& valueSet, bool retain)
            :CollectionRequest(name)
            , valueSet(valueSet)
            , retain(retain) {

            }

            int CollectionCompareAndRemoveRequest::getClassId() const {
                return CollectionPortableHook::COLLECTION_COMPARE_AND_REMOVE;
            }

            void CollectionCompareAndRemoveRequest::writePortable(serialization::PortableWriter& writer) const {
                CollectionRequest::writePortable(writer);
                writer.writeBoolean("r", retain);
                serialization::ObjectDataOutput & output = writer.getRawDataOutput();
                output.writeInt(valueSet.size());
                for (int i = 0; i < valueSet.size(); i++) {
                    valueSet[i].writeData(output);
                }

            }


        }
    }
}