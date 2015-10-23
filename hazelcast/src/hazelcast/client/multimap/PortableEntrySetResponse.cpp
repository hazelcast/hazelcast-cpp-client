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
// Created by sancar koyunlu on 9/12/13.



#include "hazelcast/client/multimap/PortableEntrySetResponse.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            PortableEntrySetResponse::PortableEntrySetResponse() {

            }

            const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > >& PortableEntrySetResponse::getEntrySet() const {
                return entrySet;
            }

            int PortableEntrySetResponse::getFactoryId() const {
                return collection::CollectionPortableHook::F_ID;
            }

            int PortableEntrySetResponse::getClassId() const {
                return MultiMapPortableHook::ENTRY_SET_RESPONSE;
            }

            void PortableEntrySetResponse::readPortable(serialization::PortableReader& reader) {
                int size = reader.readInt("s");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                for (int i = 0; i < size; ++i) {
                    serialization::pimpl::Data keyData = in.readData();
                    serialization::pimpl::Data valueData = in.readData();
                    entrySet.push_back(std::make_pair(keyData, valueData));
                }
            }
        }

    }
}



