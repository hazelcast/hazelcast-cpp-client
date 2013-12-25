//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_Request
#define HAZELCAST_Request

#include "hazelcast/client/Portable.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            class HAZELCAST_API PortableRequest : public Portable {
            public:

                ~PortableRequest();

                virtual void writePortable(serialization::PortableWriter& writer) const;

                /* final */ void readPortable(serialization::PortableReader &reader);
            };
        }
    }
}

#endif //HAZELCAST_Request
