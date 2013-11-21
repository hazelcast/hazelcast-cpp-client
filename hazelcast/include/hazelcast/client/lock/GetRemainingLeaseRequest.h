//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetRemainingLeaseRequest
#define HAZELCAST_GetRemainingLeaseRequest


#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/impl/RetryableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace lock {
            class GetRemainingLeaseRequest : public impl::PortableRequest, public RetryableRequest {
            public:
                GetRemainingLeaseRequest(serialization::Data& key);

                int getClassId() const;

                int getFactoryId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                serialization::Data& key;
            };
        }
    }
}


#endif //HAZELCAST_GetRemainingLeaseRequest
