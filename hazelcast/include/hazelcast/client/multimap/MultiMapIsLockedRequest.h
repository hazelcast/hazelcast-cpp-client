//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MultiMapIsLockedRequest
#define HAZELCAST_MultiMapIsLockedRequest


#include "hazelcast/client/impl/RetryableRequest.h"
#include "hazelcast/client/multimap/KeyBasedRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace multimap {
            class HAZELCAST_API MultiMapIsLockedRequest : public KeyBasedRequest, public RetryableRequest {
            public:
                MultiMapIsLockedRequest(const std::string& name, const serialization::Data& key);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            };

        }
    }
}
#endif //HAZELCAST_MultiMapIsLockedRequest
