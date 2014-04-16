//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ADD_MLR
#define HAZELCAST_ADD_MLR

#include "hazelcast/client/impl/PortableRequest.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class HAZELCAST_API AddMembershipListenerRequest  : public impl::PortableRequest {
            public:
                AddMembershipListenerRequest();

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;
            };
        }
    }
}

#endif //HAZELCAST_ADD_MLR

