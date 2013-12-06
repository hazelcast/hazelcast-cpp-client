//
// Created by sancar koyunlu on 6/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GET_PARTITIONS_REQUEST
#define HAZELCAST_GET_PARTITIONS_REQUEST

#include "hazelcast/client/IdentifiedDataSerializable.h"
#include "hazelcast/client/impl/RetryableRequest.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"
#include "hazelcast/client/serialization/SerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ObjectDataInput;

            class ObjectDataOutput;
        }
        namespace impl {


            class HAZELCAST_API GetPartitionsRequest : public IdentifiedDataSerializable, public RetryableRequest {
            public:
                GetPartitionsRequest();

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::ObjectDataOutput& writer) const;

                void readData(serialization::ObjectDataInput& reader);

            };
        }
    }
}
#endif //HAZELCAST_GET_PARTITIONS_REQUEST
