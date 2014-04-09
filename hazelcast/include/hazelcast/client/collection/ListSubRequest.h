//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ListSubRequest
#define HAZELCAST_ListSubRequest

#include "hazelcast/client/collection/CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace list {
            class HAZELCAST_API ListSubRequest : public collection::CollectionRequest {
            public:
                ListSubRequest(const std::string &name, const std::string &serviceName, int from, int to);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                int from;
                int to;

            };
        }
    }
}

#endif //HAZELCAST_ListSubRequest

