//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetLockCountRequest
#define HAZELCAST_GetLockCountRequest

#include "hazelcast/client/impl/RetryableRequest.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace lock {
            class HAZELCAST_API GetLockCountRequest : public impl::PortableRequest, public RetryableRequest {
            public:
                GetLockCountRequest(serialization::Data& key);

                int getClassId() const;

                int getFactoryId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                serialization::Data& key;
            };
        }
    }
}


#endif //HAZELCAST_GetLockCountRequest
