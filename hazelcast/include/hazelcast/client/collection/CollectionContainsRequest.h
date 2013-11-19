//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_CollectionContainsRequest
#define HAZELCAST_CollectionContainsRequest

#include "hazelcast/client/collection/CollectionRequest.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace collection {
            class CollectionContainsRequest : public CollectionRequest {
            public:
                CollectionContainsRequest(const std::string& name, const std::vector<serialization::Data>& valueSet);

                void writePortable(serialization::PortableWriter& writer) const;

                int getClassId() const;

            private:
                const std::vector<serialization::Data>& valueSet;

            };
        }
    }
}

#endif //HAZELCAST_CollectionContainsRequest
