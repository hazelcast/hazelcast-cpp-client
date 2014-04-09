//
// Created by sancar koyunlu on 6/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GET_PARTITIONS_REQUEST
#define HAZELCAST_GET_PARTITIONS_REQUEST

#include "hazelcast/client/protocol/ProtocolConstants.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/client/impl/PortableRequest.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class HAZELCAST_API GetPartitionsRequest : public impl::PortableRequest{
            public:
                GetPartitionsRequest();

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

                bool isRetryable() const;
            };
        }
    }
}
#endif //HAZELCAST_GET_PARTITIONS_REQUEST

