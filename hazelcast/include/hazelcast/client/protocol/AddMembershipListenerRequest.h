//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ADD_MLR
#define HAZELCAST_ADD_MLR

#include "hazelcast/client/serialization/SerializationConstants.h"
#include "hazelcast/client/impl/IdentifiedDataSerializableRequest.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class AddMembershipListenerRequest : public impl::IdentifiedDataSerializableRequest {
            public:
                AddMembershipListenerRequest();

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::ObjectDataOutput& writer) const;
            };
        }
    }
}

#endif //HAZELCAST_ADD_MLR
