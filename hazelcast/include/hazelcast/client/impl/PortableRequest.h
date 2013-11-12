//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_Request
#define HAZELCAST_Request

#include "Portable.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            class PortableRequest : public Portable {
            public:

                ~PortableRequest();

                /* final */ void readPortable(serialization::PortableReader &reader);
            };
        }
    }
}

#endif //HAZELCAST_Request
