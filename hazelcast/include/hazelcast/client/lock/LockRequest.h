//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_Lock_LockRequest
#define HAZELCAST_Lock_LockRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }
        namespace lock {
            class HAZELCAST_API LockRequest : public impl::PortableRequest {
            public:

                LockRequest(serialization::pimpl::Data &key, long threadId, long ttl, long timeout);

                int getClassId() const;

                int getFactoryId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                serialization::pimpl::Data &key;
                long threadId;
                long ttl;
                long timeout;
            };
        }
    }
}


#endif //HAZELCAST_LockRequest
