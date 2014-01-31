//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ContainsEntryRequest
#define HAZELCAST_ContainsEntryRequest

#include "hazelcast/client/multimap/AllPartitionsRequest.h"
#include "hazelcast/client/serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API ContainsEntryRequest : public AllPartitionsRequest {
            public:
                ContainsEntryRequest(serialization::Data &key, const std::string &name, serialization::Data &value);

                ContainsEntryRequest(serialization::Data &key, const std::string &name);

                ContainsEntryRequest(const std::string &name, serialization::Data &value);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

                bool isRetryable() const;

            private:
                bool hasKey;
                bool hasValue;
                serialization::Data key;
                serialization::Data value;
            };
        }
    }
}

#endif //HAZELCAST_ContainsEntryRequest
