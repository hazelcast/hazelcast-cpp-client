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
// Created by sancar koyunlu on 5/23/13.

#ifndef HAZELCAST_MAP_PUT_ALL_REQUEST
#define HAZELCAST_MAP_PUT_ALL_REQUEST

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/impl/MapEntrySet.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class PutAllRequest : public impl::ClientRequest {
            public:
                PutAllRequest(const std::string& name,
                              const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >& m,
                              int pId);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                std::string name;
                map::MapEntrySet entrySet;
                int partitionId;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_ALL_REQUEST

