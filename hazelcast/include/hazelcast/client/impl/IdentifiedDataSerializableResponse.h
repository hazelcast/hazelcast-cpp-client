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
// Created by sancar koyunlu on 19/11/13.




#ifndef HAZELCAST_IdentifiedDataSerializableResponse
#define HAZELCAST_IdentifiedDataSerializableResponse

#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class HAZELCAST_API IdentifiedDataSerializableResponse : public serialization::IdentifiedDataSerializable {
            public:
                virtual ~IdentifiedDataSerializableResponse();

                /* final */ void writeData(serialization::ObjectDataOutput &writer) const;
            };

        }
    }
}


#endif //HAZELCAST_IdentifiedDataSerializableRequest
