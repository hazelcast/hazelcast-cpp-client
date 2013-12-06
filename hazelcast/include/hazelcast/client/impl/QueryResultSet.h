//
// Created by sancar koyunlu on 9/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_QueryResultSet
#define HAZELCAST_QueryResultSet

#include "hazelcast/client/IdentifiedDataSerializable.h"
#include "hazelcast/client/impl/QueryResultEntry.h"
#include <string>
#include <vector>

namespace hazelcast {
    namespace client {
        namespace map {
            class QueryResultEntry;
        }
        namespace impl {
            class HAZELCAST_API QueryResultSet : public IdentifiedDataSerializable {
            public:
                QueryResultSet();

                QueryResultSet(const std::string& name, const std::string& iterationType, const std::string& sql);

                int getFactoryId() const;

                int getClassId() const;

                const std::vector<QueryResultEntry>& getResultData() const;

                void writeData(serialization::ObjectDataOutput& out) const;

                void readData(serialization::ObjectDataInput& in);

            private:
                std::string iterationType;
                std::vector<QueryResultEntry> q;
                bool data;
            };
        }
    }
}


#endif //HAZELCAST_QueryResultSet
