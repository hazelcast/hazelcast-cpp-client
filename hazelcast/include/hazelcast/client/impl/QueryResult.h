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



#ifndef HAZELCAST_QueryResultSet
#define HAZELCAST_QueryResultSet

#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"
#include <string>
#include <vector>

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl{
                 class Data;
            }
        }
        namespace impl {
            class QueryResult : public impl::IdentifiedDataSerializableResponse {
            public:
                int getFactoryId() const;

                int getClassId() const;

                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > getResultData() const;

                void readData(serialization::ObjectDataInput& in);

            private:
                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > q;
            };
        }
    }
}


#endif //HAZELCAST_QueryResultSet

