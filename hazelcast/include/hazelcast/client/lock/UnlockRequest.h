//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ILock_UnlockRequest
#define HAZELCAST_ILock_UnlockRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace lock {
            class HAZELCAST_API UnlockRequest : public impl::PortableRequest {
            public:
                UnlockRequest(serialization::Data& key, int threadId);

                UnlockRequest(serialization::Data& key, int threadId, bool force);

                int getClassId() const;

                int getFactoryId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                serialization::Data& key;
                int threadId;
                bool force;
            };
        }
    }
}

#endif //HAZELCAST_UnlockRequest
