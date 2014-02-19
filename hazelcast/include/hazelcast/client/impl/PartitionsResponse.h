//
// Created by sancar koyunlu on 6/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PARTITION_RESPONSE
#define HAZELCAST_PARTITION_RESPONSE

#include "hazelcast/client/Address.h"
#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace impl {
            class HAZELCAST_API PartitionsResponse : public impl::IdentifiedDataSerializableResponse {
            public:
                const std::vector<Address>& getMembers() const;

                const std::vector<int>& getOwnerIndexes() const;

                int getFactoryId() const;

                int getClassId() const;

                void readData(serialization::ObjectDataInput& reader);

            private:
                std::vector<Address> members;
                std::vector<int> ownerIndexes;
            };

        }
    }
}
#endif //HAZELCAST_PARTITION_RESPONSE
