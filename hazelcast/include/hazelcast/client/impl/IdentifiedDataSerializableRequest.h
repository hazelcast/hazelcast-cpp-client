//
// Created by sancar koyunlu on 12/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_IdentifiedDataSerializableRequest
#define HAZELCAST_IdentifiedDataSerializableRequest

#include "hazelcast/client/IdentifiedDataSerializable.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            class IdentifiedDataSerializableRequest : public IdentifiedDataSerializable {
            public:

                ~IdentifiedDataSerializableRequest();

                /* final */ void readData(serialization::ObjectDataInput &reader);
            };

        }
    }
}
#endif //HAZELCAST_IdentifiedDataSerializableRequest
