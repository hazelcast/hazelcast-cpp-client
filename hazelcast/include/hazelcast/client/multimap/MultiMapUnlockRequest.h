//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MultiMapUnlockRequest
#define HAZELCAST_MultiMapUnlockRequest

#include "hazelcast/client/multimap/KeyBasedRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API MultiMapUnlockRequest : public KeyBasedRequest {
            public:
                MultiMapUnlockRequest(const std::string &name, const serialization::pimpl::Data &key, long threadId);

                MultiMapUnlockRequest(const std::string &name, const serialization::pimpl::Data &key, long threadId, bool force);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                long threadId;
                bool force;
            };

        }
    }
}


#endif //HAZELCAST_MultiMapUnlockRequest
