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
// Created by sancar koyunlu on 6/21/13.




#ifndef HAZELCAST_ADD_ENTRY_LISTENER_REQUEST
#define HAZELCAST_ADD_ENTRY_LISTENER_REQUEST

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {

            class AddEntryListenerRequest : public impl::ClientRequest {
            public:
                AddEntryListenerRequest(const std::string &name, bool includeValue);

                AddEntryListenerRequest(const std::string &name, bool includeValue, const serialization::pimpl::Data &key, const std::string &sql);

                AddEntryListenerRequest(const std::string &name, bool includeValue, const std::string &sql);

                AddEntryListenerRequest(const std::string &name, bool includeValue, const serialization::pimpl::Data &key);

                int getFactoryId() const;

                int getClassId() const;

                const serialization::pimpl::Data *getKey() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                static int ALL_LISTENER_FLAGS;
                std::string name;
                bool includeValue;
                serialization::pimpl::Data key;
                std::string sql;
                bool hasKey;
                bool hasPredicate;
            };

        }
    }
}

#endif //HAZELCAST_ADD_ENTRY_LISTENER_REQUEST

