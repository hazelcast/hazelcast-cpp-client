//
// Created by sancar koyunlu on 9/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_QueryResultEntry
#define HAZELCAST_QueryResultEntry

#include "IdentifiedDataSerializable.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class QueryResultEntry : public IdentifiedDataSerializable {
            public:
                QueryResultEntry();

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::ObjectDataOutput& writer) const;

                void readData(serialization::ObjectDataInput& reader);

                serialization::Data keyIndex;
                serialization::Data key;
                serialization::Data value;
            };
        }
    }
}


#endif //HAZELCAST_QueryResultEntry
