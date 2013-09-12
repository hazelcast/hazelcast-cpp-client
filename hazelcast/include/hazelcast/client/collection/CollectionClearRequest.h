//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_CollectionClearRequest
#define HAZELCAST_CollectionClearRequest

#include "CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class CollectionClearRequest : public CollectionRequest {
            public:
                CollectionClearRequest(const std::string& name);

                int getClassId() const;

            };
        }
    }
}

#endif //HAZELCAST_CollectionClearRequest
