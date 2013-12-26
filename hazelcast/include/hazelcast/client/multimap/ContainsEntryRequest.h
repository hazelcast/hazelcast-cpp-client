//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ContainsEntryRequest
#define HAZELCAST_ContainsEntryRequest

#include "hazelcast/client/impl/RetryableRequest.h"
#include "hazelcast/client/multimap/AllPartitionsRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace multimap {
            class HAZELCAST_API ContainsEntryRequest : public AllPartitionsRequest, public RetryableRequest {
            public:
                ContainsEntryRequest(const serialization::Data& key, const std::string& name, const serialization::Data& value);

                ContainsEntryRequest(const serialization::Data& key, const std::string& name);

                ContainsEntryRequest(const std::string& name, const serialization::Data& value);

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                const serialization::Data *key;
                const serialization::Data *value;
            };
        }
    }
}

#endif //HAZELCAST_ContainsEntryRequest
