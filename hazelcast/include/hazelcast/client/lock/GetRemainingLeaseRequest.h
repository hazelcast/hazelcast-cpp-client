//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetRemainingLeaseRequest
#define HAZELCAST_GetRemainingLeaseRequest


#include "hazelcast/client/Portable.h"
#include "hazelcast/client/impl/RetryableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace lock {
            class GetRemainingLeaseRequest : public Portable, public RetryableRequest {
            public:
                GetRemainingLeaseRequest(serialization::Data& key);

                int getClassId() const;

                int getFactoryId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::Data& key;
            };
        }
    }
}


#endif //HAZELCAST_GetRemainingLeaseRequest
