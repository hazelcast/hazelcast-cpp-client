//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_PEEK_REQUEST
#define HAZELCAST_QUEUE_PEEK_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class HAZELCAST_API PeekRequest : public impl::PortableRequest {
            public:
                PeekRequest(const std::string &name);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

                bool isRetryable() const;

            private:
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST

