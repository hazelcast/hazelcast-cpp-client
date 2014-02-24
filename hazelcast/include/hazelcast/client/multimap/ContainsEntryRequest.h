//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ContainsEntryRequest
#define HAZELCAST_ContainsEntryRequest

#include "hazelcast/client/multimap/AllPartitionsRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API ContainsEntryRequest : public AllPartitionsRequest {
            public:
                ContainsEntryRequest(serialization::pimpl::Data &key, const std::string &name, serialization::pimpl::Data &value);

                ContainsEntryRequest(serialization::pimpl::Data &key, const std::string &name);

                ContainsEntryRequest(const std::string &name, serialization::pimpl::Data &value);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

                bool isRetryable() const;

            private:
                bool hasKey;
                bool hasValue;
                serialization::pimpl::Data key;
                serialization::pimpl::Data value;
            };
        }
    }
}

#endif //HAZELCAST_ContainsEntryRequest
