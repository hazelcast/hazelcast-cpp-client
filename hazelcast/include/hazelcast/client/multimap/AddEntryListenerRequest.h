//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AddEntryListenerRequest
#define HAZELCAST_AddEntryListenerRequest

#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API AddEntryListenerRequest : public impl::PortableRequest {
            public:
                AddEntryListenerRequest(const std::string &name, const serialization::Data &key, bool includeValue);

                AddEntryListenerRequest(const std::string &name, bool includeValue);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

                bool isRetryable() const;

            private:
                std::string name;
                serialization::Data key;
                bool hasKey;
                bool includeValue;
            };

        }
    }
}


#endif //HAZELCAST_AddEntryListenerRequest
