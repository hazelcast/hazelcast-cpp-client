//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_CLIENT_PING_REQUEST
#define HAZELCAST_CLIENT_PING_REQUEST


#include "ProtocolConstants.h"
#include "../serialization/SerializationConstants.h"
#include "IdentifiedDataSerializable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ObjectDataOutput;

            class ObjectDataInput;
        }
        namespace protocol {
            class PingRequest : public IdentifiedDataSerializable {
            public:
                PingRequest();

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::ObjectDataOutput& writer) const;

                void readData(serialization::ObjectDataInput& reader);

            };

        }
    }
}

#endif //HAZELCAST_CLIENT_PING_REQUEST
