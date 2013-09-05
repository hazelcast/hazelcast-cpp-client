//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_IsLockedRequest
#define HAZELCAST_IsLockedRequest

#include "Portable.h"
#include "RetryableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace lock {
            class IsLockedRequest : public Portable, public RetryableRequest {
            public:
                IsLockedRequest(serialization::Data& key);

                IsLockedRequest(serialization::Data& key, int threadId);

                int getClassId() const;

                int getFactoryId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                int threadId;
                serialization::Data& key;
            };
        }
    }
}


#endif //HAZELCAST_IsLockedRequest
