//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_LockRequest
#define HAZELCAST_LockRequest


#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "LockPortableHook.h"
#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace lock {
            class LockRequest : public Portable {
            public:
            public:
                LockRequest(const serialization::Data& key, int threadId);

                LockRequest(const serialization::Data& key, int threadId, long ttl, long timeout);

                int getClassId() const;

                int getFactoryId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::Data key;
                int threadId;
                long ttl;
                long timeout;
            };
        }
    }
}


#endif //HAZELCAST_LockRequest
