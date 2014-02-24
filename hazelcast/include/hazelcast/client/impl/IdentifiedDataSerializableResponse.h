//
// Created by sancar koyunlu on 19/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_IdentifiedDataSerializableResponse
#define HAZELCAST_IdentifiedDataSerializableResponse

#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class HAZELCAST_API IdentifiedDataSerializableResponse : public serialization::IdentifiedDataSerializable {
            public:
                ~IdentifiedDataSerializableResponse();

                /* final */ void writeData(serialization::ObjectDataOutput &writer) const;
            };

        }
    }
}


#endif //HAZELCAST_IdentifiedDataSerializableRequest