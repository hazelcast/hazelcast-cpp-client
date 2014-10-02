//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MultiMapLockRequest
#define HAZELCAST_MultiMapLockRequest

#include "hazelcast/client/multimap/KeyBasedRequest.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace multimap {
            class MultiMapLockRequest : public KeyBasedRequest {
            public:
                MultiMapLockRequest(const std::string &name, const serialization::pimpl::Data &key, long threadId);

                MultiMapLockRequest(const std::string &name, const serialization::pimpl::Data &key, long threadId, long ttl, long timeout);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                long threadId;
                long ttl;
                long timeout;
            };

        }
    }
}

#endif //HAZELCAST_MultiMapLockRequest

