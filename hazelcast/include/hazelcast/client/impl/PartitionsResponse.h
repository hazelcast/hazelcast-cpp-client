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
// Created by sancar koyunlu on 6/4/13.




#ifndef HAZELCAST_PARTITION_RESPONSE
#define HAZELCAST_PARTITION_RESPONSE

#include "hazelcast/client/Address.h"
#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace impl {
            class PartitionsResponse : public impl::IdentifiedDataSerializableResponse {
            public:
                const std::vector<Address>& getMembers() const;

                const std::vector<int>& getOwnerIndexes() const;

                int getFactoryId() const;

                int getClassId() const;

                void readData(serialization::ObjectDataInput& reader);

            private:
                std::vector<Address> members;
                std::vector<int> ownerIndexes;
            };

        }
    }
}
#endif //HAZELCAST_PARTITION_RESPONSE

