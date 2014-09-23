//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetRemainingLeaseRequest
#define HAZELCAST_GetRemainingLeaseRequest


#include "hazelcast/client/impl/ClientRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }
        namespace lock {
            class HAZELCAST_API GetRemainingLeaseRequest : public impl::ClientRequest{
            public:
                GetRemainingLeaseRequest(serialization::pimpl::Data& key);

                int getClassId() const;

                int getFactoryId() const;

                void write(serialization::PortableWriter& writer) const;

                bool isRetryable() const;
            private:
                serialization::pimpl::Data& key;
            };
        }
    }
}


#endif //HAZELCAST_GetRemainingLeaseRequest

