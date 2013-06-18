//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "MapEntrySet.h"
#include "../map/ResponseIDs.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            MapEntrySet::MapEntrySet() {

            }

            int MapEntrySet::getTypeSerializerId() const {
                return serialization::SerializationConstants::CONSTANT_TYPE_DATA;
            }

            int MapEntrySet::getFactoryId() const {
                return map::ResponseIDs::F_ID;
            }

            int MapEntrySet::getClassId() const {
                return map::ResponseIDs::ENTRY_SET;
            }

            const std::vector< std::pair< serialization::Data, serialization::Data> >  & MapEntrySet::getEntrySet() const {
                return entrySet;
            }

            std::vector< std::pair< serialization::Data, serialization::Data> >  & MapEntrySet::getEntrySet() {
                return entrySet;
            }
        }
    }
}