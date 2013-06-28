//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "MapEntrySet.h"
#include "DataSerializableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            MapEntrySet::MapEntrySet() {

            }

            int MapEntrySet::getTypeSerializerId() const {
                return serialization::SerializationConstants::CONSTANT_TYPE_DATA;
            }

            int MapEntrySet::getFactoryId() const {
                return DataSerializableHook::F_ID;
            }

            int MapEntrySet::getClassId() const {
                return DataSerializableHook::ENTRY_SET;
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