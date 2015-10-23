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
// Created by sancar koyunlu on 9/2/13.





#ifndef HAZELCAST_QueryResultEntry
#define HAZELCAST_QueryResultEntry

#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class HAZELCAST_API QueryResultRow : public impl::IdentifiedDataSerializableResponse {
            public:
                int getFactoryId() const;

                int getClassId() const;

                void readData(serialization::ObjectDataInput& reader);

                serialization::pimpl::Data key;
                serialization::pimpl::Data value;
            };
        }
    }
}


#endif //HAZELCAST_QueryResultEntry

