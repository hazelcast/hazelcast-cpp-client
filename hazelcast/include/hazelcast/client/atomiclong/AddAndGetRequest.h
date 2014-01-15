//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_AddAndGetRequest
#define HAZELCAST_AddAndGetRequest

#include "hazelcast/client/atomiclong/AtomicLongRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class PortableWriter;
        }

        namespace atomiclong {

            class HAZELCAST_API AddAndGetRequest : public AtomicLongRequest {
            public:
                AddAndGetRequest(const std::string &instanceName, long delta);

                int getClassId() const;

            };
        }
    }
}

#endif //HAZELCAST_AddAndGetRequest
