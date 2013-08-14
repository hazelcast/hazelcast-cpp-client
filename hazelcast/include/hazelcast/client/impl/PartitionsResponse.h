//
// Created by sancar koyunlu on 6/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PARTITION_RESPONSE
#define HAZELCAST_PARTITION_RESPONSE

#include "../Address.h"
#include "IdentifiedDataSerializable.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace impl {
            class PartitionsResponse : public IdentifiedDataSerializable {
            public:
                PartitionsResponse();

                PartitionsResponse(std::vector<Address>& addresses, std::vector<int>& ownerIndexes);

                const std::vector<Address>& getMembers() const;

                const std::vector<int>& getOwnerIndexes() const;

                bool isEmpty();

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::ObjectDataOutput& writer) const;

                void readData(serialization::ObjectDataInput& reader);

            private:
                std::vector<Address> members;
                std::vector<int> ownerIndexes;
            };

        }
    }
}
#endif //HAZELCAST_PARTITION_RESPONSE
