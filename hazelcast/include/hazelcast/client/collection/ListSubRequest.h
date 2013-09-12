//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ListSubRequest
#define HAZELCAST_ListSubRequest

#include "CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace list {
            class ListSubRequest : public collection::CollectionRequest {
            public:
                ListSubRequest(const std::string& name, int from, int to);

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                int from;
                int to;

            };
        }
    }
}

#endif //HAZELCAST_ListSubRequest
