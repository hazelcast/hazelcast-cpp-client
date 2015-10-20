//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PORTABLE_ITEM_EVENT
#define HAZELCAST_PORTABLE_ITEM_EVENT

#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/impl/PortableResponse.h"
#include "hazelcast/client/ItemEvent.h"
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            class HAZELCAST_API PortableItemEvent : public impl::PortableResponse {
            public:

                const serialization::pimpl::Data&  getItem() const;

                std::string &getUuid() const;

                ItemEventType getEventType() const;

                int getFactoryId() const;

                int getClassId() const;

                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::pimpl::Data item;
                ItemEventType eventType;
                std::auto_ptr<std::string> uuid;

            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_PORTABLE_ITEM_EVENT

