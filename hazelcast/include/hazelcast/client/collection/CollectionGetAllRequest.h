//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_CollectionGetAllRequest
#define HAZELCAST_CollectionGetAllRequest

#include "hazelcast/client/collection/CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class CollectionGetAllRequest : public CollectionRequest {
            public:
                CollectionGetAllRequest(const std::string& name);

                int getClassId() const;

            };
        }
    }
}

#endif //HAZELCAST_CollectionGetAllRequest
