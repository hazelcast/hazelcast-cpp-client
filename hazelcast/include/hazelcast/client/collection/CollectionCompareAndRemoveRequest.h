//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CollectionCompareAndRemoveRequest
#define HAZELCAST_CollectionCompareAndRemoveRequest

#include "hazelcast/client/collection/CollectionRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace collection {
            class HAZELCAST_API CollectionCompareAndRemoveRequest : public CollectionRequest {
            public:
                CollectionCompareAndRemoveRequest(const std::string &name, const std::string &serviceName, const std::vector<serialization::pimpl::Data> &valueSet, bool retain);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::vector<serialization::pimpl::Data> valueSet;
                bool retain;

            };
        }
    }
}

#endif //HAZELCAST_CollectionCompareAndRemoveRequest

