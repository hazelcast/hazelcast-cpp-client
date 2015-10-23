/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 6/18/13.



#include "hazelcast/client/impl/MapEntrySet.h"
#include "hazelcast/client/map/DataSerializableHook.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace map {
            MapEntrySet::MapEntrySet() {

            }


            MapEntrySet::MapEntrySet(const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >& entrySet)
            :entrySet(entrySet) {

            }

            int MapEntrySet::getFactoryId() const {
                return DataSerializableHook::F_ID;
            }

            int MapEntrySet::getClassId() const {
                return DataSerializableHook::ENTRY_SET;
            }

            std::vector< std::pair< serialization::pimpl::Data, serialization::pimpl::Data> >  & MapEntrySet::getEntrySet() {
                return entrySet;
            }

            void MapEntrySet::writeData(serialization::ObjectDataOutput& writer) const {
                int size = entrySet.size();
		        writer.writeInt(size);
                for (int i = 0; i < size; ++i) {
                    writer.writeData(&(entrySet[i].first));
                    writer.writeData(&(entrySet[i].second));
                }
            }

            void MapEntrySet::readData(serialization::ObjectDataInput& reader) {
                int size = reader.readInt();
                entrySet.resize(size);
                for (int i = 0; i < size; i++) {
                    entrySet[i].first = reader.readData();;
                    entrySet[i].second = reader.readData();
                }
            }
        }
    }
}

