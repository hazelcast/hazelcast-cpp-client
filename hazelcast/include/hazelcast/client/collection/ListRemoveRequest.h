//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ListRemoveRequest
#define HAZELCAST_ListRemoveRequest

#include "hazelcast/client/collection/CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace list {
            class HAZELCAST_API ListRemoveRequest : public collection::CollectionRequest {
            public:
                ListRemoveRequest(const std::string &name, const std::string &serviceName, int index);

                void write(serialization::PortableWriter &writer) const;

                int getClassId() const;

            private:
                int index;
            };
        }
    }
}

#endif //HAZELCAST_ListRemoveRequest

