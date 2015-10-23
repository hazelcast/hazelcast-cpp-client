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
// Created by sancar koyunlu on 5/29/13.




#ifndef HAZELCAST_SERIALIZABLE_CONNECTION
#define HAZELCAST_SERIALIZABLE_CONNECTION

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            class SerializableCollection : public IdentifiedDataSerializableResponse {
            public:
                SerializableCollection();

                const std::vector<serialization::pimpl::Data>& getCollection() const;

                int getFactoryId() const;

                int getClassId() const;

                void readData(serialization::ObjectDataInput& reader);

            private:
                std::vector<serialization::pimpl::Data> dataCollection;
            };
        }
    }
}

#endif //HAZELCAST_SERIALIZABLE_CONNECTION

