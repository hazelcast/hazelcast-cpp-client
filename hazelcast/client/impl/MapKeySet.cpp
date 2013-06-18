//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "MapKeySet.h"
#include "../map/ResponseIDs.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            MapKeySet::MapKeySet() {

            };

            const std::vector<serialization::Data>& MapKeySet::getKeySet() const {
                return keySet;
            };

            int MapKeySet::getTypeSerializerId() const {
                return serialization::SerializationConstants::CONSTANT_TYPE_DATA;
            }

            int MapKeySet::getFactoryId() const {
                return map::ResponseIDs::F_ID;
            }

            int MapKeySet::getClassId() const {
                return map::ResponseIDs::KEY_SET;
            }
        }
    }
}