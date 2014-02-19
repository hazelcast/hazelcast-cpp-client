//
// Created by sancar koyunlu on 9/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_QueryResultEntry
#define HAZELCAST_QueryResultEntry

#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"
#include "hazelcast/client/serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class HAZELCAST_API QueryResultEntry : public impl::IdentifiedDataSerializableResponse {
            public:
                int getFactoryId() const;

                int getClassId() const;

                void readData(serialization::ObjectDataInput& reader);

                serialization::Data keyIndex;
                serialization::Data key;
                serialization::Data value;
            };
        }
    }
}


#endif //HAZELCAST_QueryResultEntry
