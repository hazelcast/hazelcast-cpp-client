//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_LOCK_REQUEST
#define HAZELCAST_LOCK_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace map {
            class HAZELCAST_API LockRequest : public impl::PortableRequest {
            public:
                LockRequest(const std::string& name, serialization::Data& key, int threadId, long ttl, long timeout);

                LockRequest(const std::string& name, serialization::Data& key, int threadId);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;



            private:
                serialization::Data& key;
                std::string name;
                int threadId;
                long ttl;
                long timeout;
            };
        }
    }
}

#endif //HAZELCAST_LOCK_REQUEST
