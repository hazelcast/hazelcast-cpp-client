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


#ifndef HAZELCAST_PORTABLE_ENTRY_EVENT
#define HAZELCAST_PORTABLE_ENTRY_EVENT

#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/impl/PortableResponse.h"
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            class HAZELCAST_API PortableEntryEvent : public impl::PortableResponse {
            public:
                const serialization::pimpl::Data &getKey() const;

                const serialization::pimpl::Data &getOldValue() const;

                const serialization::pimpl::Data &getValue() const;

                const serialization::pimpl::Data &getMergingValue() const;

                std::string & getUuid() const;

                EntryEventType getEventType() const;

                int getFactoryId() const;

                int getClassId() const;

                int getNumberOfAffectedEntries() const;

                void readPortable(serialization::PortableReader &reader);

            private:
                serialization::pimpl::Data key;
                serialization::pimpl::Data value;
                serialization::pimpl::Data oldValue;
                serialization::pimpl::Data mergingValue;
                EntryEventType eventType;
                int numberOfAffectedEntries;
                std::auto_ptr<std::string> uuid;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //__PortableEntryEvent_H_

