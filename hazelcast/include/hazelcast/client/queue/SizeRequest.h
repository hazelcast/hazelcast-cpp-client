//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_SIZE_REQUEST
#define HAZELCAST_QUEUE_SIZE_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/impl/RetryableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class HAZELCAST_API SizeRequest : public impl::PortableRequest, public RetryableRequest {
            public:
                SizeRequest(const std::string& name);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                const std::string& name;
            };
        }
    }
}

#endif //HAZELCAST_SIZE_REQUEST
