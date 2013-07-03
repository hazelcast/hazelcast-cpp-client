//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "MapValueCollection.h"
#include "DataSerializableHook.h"
#include "BufferedDataOutput.h"
#include "BufferedDataInput.h"

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

            const vector<serialization::Data>  & MapValueCollection::getValues() const {
                return values;
            };

            void MapValueCollection::writeData(serialization::BufferedDataOutput& writer) {
                writer.writeInt(values.size());
                for (int i = 0; i < values.size(); ++i) {
                    values[i].writeData(writer);
                }
            }

            void MapValueCollection::readData(serialization::BufferedDataInput& reader) {
                int size = reader.readInt();
                values.resize(size);
                for (int i = 0; i < size; ++i) {
                    values[i].readData(reader);
                }
            }


        }
    }
}