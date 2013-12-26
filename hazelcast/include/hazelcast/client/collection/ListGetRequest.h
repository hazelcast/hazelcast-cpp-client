//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ListGetRequest
#define HAZELCAST_ListGetRequest

#include "hazelcast/client/collection/CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace list {
            class HAZELCAST_API ListGetRequest : public collection::CollectionRequest {
            public:
                ListGetRequest(const std::string& name, int index);

                void write(serialization::PortableWriter& writer) const;

                int getClassId() const;

            private:
                int index;
            };
        }
    }
}

#endif //HAZELCAST_ListGetRequest
