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
            class HAZELCAST_API ContainsRequest : public AllPartitionsRequest {
            public:
                ContainsRequest(const std::string &name, serialization::pimpl::Data &value);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

                bool isRetryable() const;

            private:
                serialization::pimpl::Data value;
            };
        }
    }
}

#endif //HAZELCAST_ContainsEntryRequest

