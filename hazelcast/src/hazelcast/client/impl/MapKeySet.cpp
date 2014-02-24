//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/MapKeySet.h"
#include "hazelcast/client/map/DataSerializableHook.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace map {

            const std::vector<serialization::pimpl::Data>& MapKeySet::getKeySet() const {
                return keySet;
            };

            int MapKeySet::getFactoryId() const {
                return DataSerializableHook::F_ID;
            }

            int MapKeySet::getClassId() const {
                return DataSerializableHook::KEY_SET;
            }

            void MapKeySet::readData(serialization::ObjectDataInput& reader) {
                int size = reader.readInt();
                for (int i = 0; i < size; i++) {
                    serialization::pimpl::Data data;
                    data.readData(reader);
                    keySet.push_back(data);
                }
            }


        }
    }
}