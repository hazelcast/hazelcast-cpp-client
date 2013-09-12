//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MultiMapIsLockedRequest
#define HAZELCAST_MultiMapIsLockedRequest


#include "RetryableRequest.h"
#include "KeyBasedRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace multimap {
            class MultiMapIsLockedRequest : public KeyBasedRequest, public RetryableRequest {
            public:
                MultiMapIsLockedRequest(const std::string& name, const serialization::Data& key);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            };

        }
    }
}
#endif //HAZELCAST_MultiMapIsLockedRequest
