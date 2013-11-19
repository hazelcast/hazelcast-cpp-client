//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetAndSetRequest
#define HAZELCAST_GetAndSetRequest

#include "hazelcast/client/atomiclong/AtomicLongRequest.h"

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            class GetAndSetRequest : public AtomicLongRequest {
            public:
                GetAndSetRequest(const std::string &instanceName, long value);

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;
            };
        }
    }
}


#endif //HAZELCAST_GetAndSetRequest
