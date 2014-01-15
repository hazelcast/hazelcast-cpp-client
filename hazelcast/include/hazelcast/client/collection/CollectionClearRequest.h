//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_CollectionClearRequest
#define HAZELCAST_CollectionClearRequest

#include "hazelcast/client/collection/CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class HAZELCAST_API CollectionClearRequest : public CollectionRequest {
            public:
                CollectionClearRequest(const std::string &name, const std::string &serviceName);

                int getClassId() const;

            };
        }
    }
}

#endif //HAZELCAST_CollectionClearRequest
