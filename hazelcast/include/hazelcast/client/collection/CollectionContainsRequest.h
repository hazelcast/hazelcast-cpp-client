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





#ifndef HAZELCAST_CollectionContainsRequest
#define HAZELCAST_CollectionContainsRequest

#include "hazelcast/client/collection/CollectionRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl{
                class Data;
            }
        }
        namespace collection {
            class CollectionContainsRequest : public CollectionRequest {
            public:
                CollectionContainsRequest(const std::string &name, const std::string &serviceName, const std::vector<serialization::pimpl::Data> &valueSet);

                void write(serialization::PortableWriter &writer) const;

                int getClassId() const;

            private:
                std::vector<serialization::pimpl::Data> valueSet;

            };
        }
    }
}

#endif //HAZELCAST_CollectionContainsRequest

