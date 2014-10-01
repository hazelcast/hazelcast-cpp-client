//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CompareAndSetRequest
#define HAZELCAST_CompareAndSetRequest

#include "hazelcast/client/atomiclong/AtomicLongRequest.h"

namespace hazelcast {
    namespace client {
        namespace atomiclong {
            class CompareAndSetRequest : public AtomicLongRequest {
            public:
                CompareAndSetRequest(const std::string& instanceName, long expect, long value);

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                long expect;

            };
        }
    }
}


#endif //HAZELCAST_CompareAndSetRequest

