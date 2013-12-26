//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CollectionCompareAndRemoveRequest
#define HAZELCAST_CollectionCompareAndRemoveRequest

#include "hazelcast/client/collection/CollectionRequest.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace collection {
            class HAZELCAST_API CollectionCompareAndRemoveRequest : public CollectionRequest {
            public:
                CollectionCompareAndRemoveRequest(const std::string& name, const std::vector<serialization::Data> & valueSet, bool retain);

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                const std::vector<serialization::Data> & valueSet;
                bool retain;

            };
        }
    }
}

#endif //HAZELCAST_CollectionCompareAndRemoveRequest
