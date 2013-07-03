//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ADD_MLR
#define HAZELCAST_ADD_MLR

#include "ProtocolConstants.h"
#include "../serialization/SerializationConstants.h"
#include "DataSerializable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class BufferedDataOutput;

            class BufferedDataInput;
        }
        namespace protocol {
            class AddMembershipListenerRequest : public DataSerializable {
            public:
                AddMembershipListenerRequest();

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::BufferedDataOutput& writer);

                void readData(serialization::BufferedDataInput& reader);

            private:
            };
        }
    }
}

#endif //HAZELCAST_ADD_MLR
