//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_Lock_DestroyRequest
#define HAZELCAST_Lock_DestroyRequest

#include "RetryableRequest.h"
#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace lock {
            class DestroyRequest : public Portable, public RetryableRequest {
            public:
                DestroyRequest(serialization::Data& key);

                int getClassId() const;

                int getFactoryId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::Data& key;
            };
        }
    }
}


#endif //HAZELCAST_DestroyRequest
