//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CountDownLatch_DestroyRequest
#define HAZELCAST_CountDownLatch_DestroyRequest

#include "RetryableRequest.h"
#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace countdownlatch {
            class DestroyRequest : public Portable, public RetryableRequest {
            public:
                DestroyRequest(const std::string& instanceName);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::string instanceName;
            };
        }
    }
}


#endif //HAZELCAST_DestroyRequest
