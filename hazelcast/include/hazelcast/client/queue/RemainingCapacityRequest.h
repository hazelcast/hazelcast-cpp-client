//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_REMAINING_CAPACITY_REQUEST
#define HAZELCAST_QUEUE_REMAINING_CAPACITY_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/impl/RetryableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class HAZELCAST_API RemainingCapacityRequest : public impl::PortableRequest, public RetryableRequest {
            public:
                RemainingCapacityRequest(const std::string& name);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                const std::string& name;
            };
        }
    }
}

#endif //HAZELCAST_REMAINING_CAPACITY_REQUEST
