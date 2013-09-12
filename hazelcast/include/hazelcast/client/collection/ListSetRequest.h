//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ListSetRequest
#define HAZELCAST_ListSetRequest

#include "CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace list {
            class ListSetRequest : public collection::CollectionRequest {
            public:

                ListSetRequest(const std::string& name, const serialization::Data& data, bool last);

                void writePortable(serialization::PortableWriter& writer) const;

                int getClassId() const;

            private:
                const serialization::Data& data;
                bool last;
            };
        }
    }
}

#endif //HAZELCAST_ListSetRequest
