//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ADD_ENTRY_LISTENER_REQUEST
#define HAZELCAST_ADD_ENTRY_LISTENER_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {

            class HAZELCAST_API AddEntryListenerRequest : public impl::PortableRequest {
            public:
                AddEntryListenerRequest(const std::string& name, bool includeValue);

                AddEntryListenerRequest(const std::string& name, bool includeValue, const serialization::Data& key, const std::string& sql);

                AddEntryListenerRequest(const std::string& name, bool includeValue, const std::string& sql);

                AddEntryListenerRequest(const std::string& name, bool includeValue, const serialization::Data& key);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;



            private:
                std::string name;
                std::string sql;
                serialization::Data key;
                bool includeValue;
                bool hasKey;
                bool hasPredicate;
            };

        }
    }
}

#endif //HAZELCAST_ADD_ENTRY_LISTENER_REQUEST
