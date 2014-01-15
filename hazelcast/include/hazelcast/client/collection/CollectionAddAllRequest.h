//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_CollectionAddAllRequest
#define HAZELCAST_CollectionAddAllRequest

#include "hazelcast/client/collection/CollectionRequest.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace collection {

            class HAZELCAST_API CollectionAddAllRequest : public CollectionRequest {
            public:
                CollectionAddAllRequest(const std::string &name, const std::string &serviceName, const std::vector<serialization::Data> &valueList);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                const std::vector<serialization::Data> &valueList;
            };
        }
    }
}

#endif //HAZELCAST_CollectionAddAllRequest
