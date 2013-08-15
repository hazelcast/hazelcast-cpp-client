//
// Created by sancar koyunlu on 6/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GET_PARTITIONS_REQUEST
#define HAZELCAST_GET_PARTITIONS_REQUEST

#include "IdentifiedDataSerializable.h"
#include "../protocol/ProtocolConstants.h"
#include "../serialization/SerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ObjectDataInput;

            class ObjectDataOutput;
        }
        namespace impl {


            class GetPartitionsRequest : public IdentifiedDataSerializable {
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
