//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_IsLockedRequest
#define HAZELCAST_IsLockedRequest

#include "hazelcast/client/impl/ClientRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }
        namespace lock {
            class HAZELCAST_API IsLockedRequest : public impl::ClientRequest {
            public:
                IsLockedRequest(serialization::pimpl::Data &key);

                IsLockedRequest(serialization::pimpl::Data &key, long threadId);

                int getClassId() const;

                int getFactoryId() const;

                void write(serialization::PortableWriter &writer) const;

                bool isRetryable() const;

            private:
                long threadId;
                serialization::pimpl::Data &key;
            };
        }
    }
}


#endif //HAZELCAST_IsLockedRequest

