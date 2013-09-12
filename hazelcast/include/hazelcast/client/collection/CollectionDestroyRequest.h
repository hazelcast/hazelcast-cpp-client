//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_CollectionDestroyRequest
#define HAZELCAST_CollectionDestroyRequest

#include "CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class CollectionDestroyRequest : public CollectionRequest {
            public:
                CollectionDestroyRequest(const std::string&name);

                int getClassId() const;
            };

        }
    }
}

#endif //HAZELCAST_CollectionDestroyRequest
