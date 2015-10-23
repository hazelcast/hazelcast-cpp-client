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
// Created by sancar koyunlu on 8/5/13.





#ifndef HAZELCAST_TxnMultiMapPutRequest
#define HAZELCAST_TxnMultiMapPutRequest

#include "hazelcast/client/multimap/TxnMultiMapRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class TxnMultiMapPutRequest : public TxnMultiMapRequest {
            public:
                TxnMultiMapPutRequest(const std::string &name, const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                serialization::pimpl::Data key;
                serialization::pimpl::Data value;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapPutRequest

