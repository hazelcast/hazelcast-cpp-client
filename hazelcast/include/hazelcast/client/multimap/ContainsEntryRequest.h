//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ContainsEntryRequest
#define HAZELCAST_ContainsEntryRequest

#include "RetryableRequest.h"
#include "AllPartitionsRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace multimap {
            class ContainsEntryRequest : public AllPartitionsRequest, public RetryableRequest {
            public:
                ContainsEntryRequest(const std::string& name, const serialization::Data& key, const serialization::Data& value);

                ContainsEntryRequest(const std::string& name, const serialization::Data& value);

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                const serialization::Data *key;
                const serialization::Data& value;
            };
        }
    }
}

#endif //HAZELCAST_ContainsEntryRequest
