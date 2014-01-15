//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_CollectionSizeRequest
#define HAZELCAST_CollectionSizeRequest

#include "hazelcast/client/collection/CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class HAZELCAST_API CollectionSizeRequest : public CollectionRequest {
            public:

                CollectionSizeRequest(const std::string &name, const std::string &serviceName);

                int getClassId() const;
            };
        }
    }
}


#endif //HAZELCAST_CollectionSizeRequest
