//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "MapKeySet.h"
#include "DataSerializableHook.h"
#include "BufferedDataOutput.h"

namespace hazelcast {
    namespace client {
        namespace map {
            MapKeySet::MapKeySet() {

            };

            const std::vector<serialization::Data>& MapKeySet::getKeySet() const {
                return keySet;
            };

            int MapKeySet::getFactoryId() const {
                return DataSerializableHook::F_ID;
            }

            int MapKeySet::getClassId() const {
                return DataSerializableHook::KEY_SET;
            }

            void MapKeySet::writeData(serialization::BufferedDataOutput& writer) {
                writer.writeInt(keySet.size());
                for (int i = 0; i < keySet.size(); ++i) {
                    keySet[i].writeData(writer);
                }

            }

            void MapKeySet::readData(serialization::BufferedDataInput& reader) {
                int size = reader.readInt();
                keySet.resize(size);
                for (int i = 0; i < size; i++) {
                    keySet[i].readData(reader);
                }
            }


        }
    }
}