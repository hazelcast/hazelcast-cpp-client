//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_CLIENT_PING_REQUEST
#define HAZELCAST_CLIENT_PING_REQUEST


#include "hazelcast/client/serialization/SerializationConstants.h"
#include "hazelcast/client/impl/IdentifiedDataSerializableRequest.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class HAZELCAST_API PingRequest : public impl::IdentifiedDataSerializableRequest {
            public:
                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::ObjectDataOutput &writer) const;

            };

        }
    }
}

#endif //HAZELCAST_CLIENT_PING_REQUEST
