//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "MapValueCollection.h"
#include "DataSerializableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            MapValueCollection::MapValueCollection() {

            }

            int MapValueCollection::getTypeSerializerId() const {
                return serialization::SerializationConstants::CONSTANT_TYPE_DATA;
            }

            int MapValueCollection::getFactoryId() const {
                return DataSerializableHook::F_ID;
            }

            int MapValueCollection::getClassId() const {
                return DataSerializableHook::VALUES;
            }

            const vector<serialization::Data>  & MapValueCollection::getValues() const {
                return values;
            }


        }
    }
}