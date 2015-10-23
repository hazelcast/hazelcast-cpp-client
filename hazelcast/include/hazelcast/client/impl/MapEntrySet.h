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




#ifndef HAZELCAST_MAP_ENTRY_SET
#define HAZELCAST_MAP_ENTRY_SET

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include <vector>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API MapEntrySet : public serialization::IdentifiedDataSerializable {
            public:
                MapEntrySet();

                MapEntrySet(const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > >& entrySet);

                int getFactoryId() const;

                int getClassId() const;

                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > > &getEntrySet();

                void writeData(serialization::ObjectDataOutput &writer) const;

                void readData(serialization::ObjectDataInput &reader);

            private:
                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > > entrySet;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_MAP_ENTRY_SET

