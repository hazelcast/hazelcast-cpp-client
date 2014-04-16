//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_PORTABLE_ENTRY_EVENT
#define HAZELCAST_PORTABLE_ENTRY_EVENT

#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/impl/PortableResponse.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace impl {
            class HAZELCAST_API PortableEntryEvent : public impl::PortableResponse {
            public:
                const serialization::pimpl::Data &getKey() const;

                const serialization::pimpl::Data &getOldValue() const;

                const serialization::pimpl::Data &getValue() const;

                std::string getUuid() const;

                EntryEventType getEventType() const;

                std::string getName() const;

                int getFactoryId() const;

                int getClassId() const;

                void readPortable(serialization::PortableReader &reader);

            private:
                serialization::pimpl::Data key;
                serialization::pimpl::Data value;
                serialization::pimpl::Data oldValue;
                EntryEventType eventType;
                std::string uuid;
                std::string name;

            };
        }
    }
}

#endif //__PortableEntryEvent_H_

