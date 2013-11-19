//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MultiMapUnlockRequest
#define HAZELCAST_MultiMapUnlockRequest

#include "hazelcast/client/multimap/KeyBasedRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace multimap {
            class MultiMapUnlockRequest : public KeyBasedRequest {
            public:
                MultiMapUnlockRequest(const std::string& name, const serialization::Data& key, int threadId);

                MultiMapUnlockRequest(const std::string& name, const serialization::Data& key, int threadId, bool force);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                int threadId;
                bool force;
            };

        }
    }
}


#endif //HAZELCAST_MultiMapUnlockRequest
