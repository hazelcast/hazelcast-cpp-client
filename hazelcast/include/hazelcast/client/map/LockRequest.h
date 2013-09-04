//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_LOCK_REQUEST
#define HAZELCAST_LOCK_REQUEST

#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class LockRequest : public Portable {
            public:
                LockRequest(const std::string& name, serialization::Data& key, int threadId, long ttl, long timeout);

                LockRequest(const std::string& name, serialization::Data& key, int threadId);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

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
