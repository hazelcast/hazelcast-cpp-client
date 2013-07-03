//
// Created by sancar koyunlu on 6/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GET_PARTITIONS_REQUEST
#define HAZELCAST_GET_PARTITIONS_REQUEST

#include "DataSerializable.h"
#include "../protocol/ProtocolConstants.h"
#include "../serialization/SerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class BufferedDataInput;

            class BufferedDataOutput;
        }
        namespace impl {


            class GetPartitionsRequest : public DataSerializable {
            public:
                GetPartitionsRequest();

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::BufferedDataOutput& writer) const;

                void readData(serialization::BufferedDataInput& reader);

            };
        }
    }
}
#endif //HAZELCAST_GET_PARTITIONS_REQUEST
