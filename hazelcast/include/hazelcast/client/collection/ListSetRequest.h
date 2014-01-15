//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ListSetRequest
#define HAZELCAST_ListSetRequest

#include "hazelcast/client/collection/CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace list {
            class HAZELCAST_API ListSetRequest : public collection::CollectionRequest {
            public:

                ListSetRequest(const std::string &name, const std::string &serviceName, const serialization::Data &data, int index);

                void write(serialization::PortableWriter &writer) const;

                int getClassId() const;

            private:
                const serialization::Data &data;
                int index;
            };
        }
    }
}

#endif //HAZELCAST_ListSetRequest
