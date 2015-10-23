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





#ifndef HAZELCAST_ListSubRequest
#define HAZELCAST_ListSubRequest

#include "hazelcast/client/collection/CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace list {
            class ListSubRequest : public collection::CollectionRequest {
            public:
                ListSubRequest(const std::string &name, const std::string &serviceName, int from, int to);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                int from;
                int to;

            };
        }
    }
}

#endif //HAZELCAST_ListSubRequest

