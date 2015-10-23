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
// Created by sancar koyunlu on 18/08/14.
//


#ifndef HAZELCAST_KeyBasedContainsRequest
#define HAZELCAST_KeyBasedContainsRequest


#include "hazelcast/client/multimap/KeyBasedRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class KeyBasedContainsRequest : public KeyBasedRequest {
            public:
                KeyBasedContainsRequest(const std::string& name, const serialization::pimpl::Data& key
                , const serialization::pimpl::Data& value, long threadId);

                KeyBasedContainsRequest(const std::string& name, const serialization::pimpl::Data& key, long threadId);

                void write(serialization::PortableWriter& writer) const;

                int getClassId() const;

                bool isRetryable() const;

            private:
                bool hasValue;
                serialization::pimpl::Data value;
                long threadId;
            };
        }
    }
}


#endif //HAZELCAST_KeyBasedContainsRequest
