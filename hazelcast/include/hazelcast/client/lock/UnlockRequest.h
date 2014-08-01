//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ILock_UnlockRequest
#define HAZELCAST_ILock_UnlockRequest

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
            class HAZELCAST_API UnlockRequest : public impl::ClientRequest {
            public:
                UnlockRequest(serialization::pimpl::Data &key, long threadId);

                UnlockRequest(serialization::pimpl::Data &key, long threadId, bool force);

                int getClassId() const;

                int getFactoryId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                serialization::pimpl::Data &key;
                long threadId;
                bool force;
            };
        }
    }
}

#endif //HAZELCAST_UnlockRequest

