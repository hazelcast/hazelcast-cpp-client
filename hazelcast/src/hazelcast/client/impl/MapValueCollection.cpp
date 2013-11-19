//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/MapValueCollection.h"
#include "hazelcast/client/map/DataSerializableHook.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace map {
            MapValueCollection::MapValueCollection() {

            }

            int MapValueCollection::getFactoryId() const {
                return DataSerializableHook::F_ID;
            };

            int MapValueCollection::getClassId() const {
                return DataSerializableHook::VALUES;
            };

            const std::vector<serialization::Data>  & MapValueCollection::getValues() const {
                return values;
            };

            void MapValueCollection::readData(serialization::ObjectDataInput& reader) {
                int size = reader.readInt();
                for (int i = 0; i < size; ++i) {
                    serialization::Data data;
                    data.readData(reader);
                    values.push_back(data);
                }
            }


        }
    }
}