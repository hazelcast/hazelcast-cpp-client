//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ADD_MLR
#define HAZELCAST_ADD_MLR

#include "ProtocolConstants.h"
#include "hazelcast/client/serialization/SerializationConstants.h"
#include "hazelcast/client/IdentifiedDataSerializable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ObjectDataOutput;

            class ObjectDataInput;
        }
        namespace protocol {
            class AddMembershipListenerRequest : public IdentifiedDataSerializable {
            public:
                AddMembershipListenerRequest();

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::ObjectDataOutput& writer) const;

                void readData(serialization::ObjectDataInput& reader);

            private:
            };
        }
    }
}

#endif //HAZELCAST_ADD_MLR
