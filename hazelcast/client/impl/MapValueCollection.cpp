//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "MapValueCollection.h"
#include "../map/ResponseIDs.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            MapValueCollection::MapValueCollection() {

            }

            int MapValueCollection::getTypeSerializerId() const {
                return serialization::SerializationConstants::CONSTANT_TYPE_DATA;
            }

            int MapValueCollection::getFactoryId() const {
                return map::ResponseIDs::F_ID;
            }

            int MapValueCollection::getClassId() const {
                return map::ResponseIDs::VALUES;
            }

            const vector<serialization::Data>  & MapValueCollection::getValues() const {
                return values;
            }


        }
    }
}