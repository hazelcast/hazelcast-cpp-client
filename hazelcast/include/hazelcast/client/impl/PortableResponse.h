//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_Response
#define HAZELCAST_Response


#include "hazelcast/client/Portable.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            class HAZELCAST_API PortableResponse : public Portable {
            public:

                ~PortableResponse();

                /* final */ void writePortable(serialization::PortableWriter &writer) const;
            };
        }
    }
}


#endif //HAZELCAST_Response
