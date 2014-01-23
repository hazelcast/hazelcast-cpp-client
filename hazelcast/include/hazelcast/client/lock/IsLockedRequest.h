//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_IsLockedRequest
#define HAZELCAST_IsLockedRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace lock {
            class HAZELCAST_API IsLockedRequest : public impl::PortableRequest {
            public:
                IsLockedRequest(serialization::Data &key);

                IsLockedRequest(serialization::Data &key, long threadId);

                int getClassId() const;

                int getFactoryId() const;

                void write(serialization::PortableWriter &writer) const;

                bool isRetryable() const;

            private:
                long threadId;
                serialization::Data &key;
            };
        }
    }
}


#endif //HAZELCAST_IsLockedRequest
