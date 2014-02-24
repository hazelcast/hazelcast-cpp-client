//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_CollectionContainsRequest
#define HAZELCAST_CollectionContainsRequest

#include "hazelcast/client/collection/CollectionRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl{
                class Data;
            }
        }
        namespace collection {
            class HAZELCAST_API CollectionContainsRequest : public CollectionRequest {
            public:
                CollectionContainsRequest(const std::string &name, const std::string &serviceName, const std::vector<serialization::pimpl::Data> &valueSet);

                void write(serialization::PortableWriter &writer) const;

                int getClassId() const;

            private:
                std::vector<serialization::pimpl::Data> valueSet;

            };
        }
    }
}

#endif //HAZELCAST_CollectionContainsRequest
