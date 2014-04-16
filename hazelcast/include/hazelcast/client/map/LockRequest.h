//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_LOCK_REQUEST
#define HAZELCAST_LOCK_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API LockRequest : public impl::PortableRequest {
            public:
                LockRequest(const std::string &name, serialization::pimpl::Data &key, long threadId, long ttl, long timeout);

                LockRequest(const std::string &name, serialization::pimpl::Data &key, long threadId);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::string name;
                serialization::pimpl::Data key;
                long threadId;
                long ttl;
                long timeout;
            };
        }
    }
}

#endif //HAZELCAST_LOCK_REQUEST

