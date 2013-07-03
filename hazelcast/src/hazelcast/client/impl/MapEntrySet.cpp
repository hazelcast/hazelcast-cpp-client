//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "MapEntrySet.h"
#include "DataSerializableHook.h"
#include "BufferedDataOutput.h"
#include "BufferedDataInput.h"

namespace hazelcast {
    namespace client {
        namespace map {
            MapEntrySet::MapEntrySet() {

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

            void MapEntrySet::writeData(serialization::BufferedDataOutput& writer) {
                writer.writeInt(entrySet.size());
                for (int i = 0; i < entrySet.size(); ++i) {
                    entrySet[i].first.writeData(writer);
                    entrySet[i].second.writeData(writer);
                }
            };

            void MapEntrySet::readData(serialization::BufferedDataInput& reader) {
                int size = reader.readInt();
                entrySet.resize(size);
                for (int i = 0; i < size; i++) {
                    entrySet[i].first.readData(reader);
                    entrySet[i].second.readData(reader);
                }
            };
        }
    }
}