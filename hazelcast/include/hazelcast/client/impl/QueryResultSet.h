//
// Created by sancar koyunlu on 9/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_QueryResultSet
#define HAZELCAST_QueryResultSet

#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"
#include "hazelcast/client/impl/QueryResultEntry.h"
#include <string>
#include <vector>

namespace hazelcast {
    namespace client {
        namespace map {
            class QueryResultEntry;
        }
        namespace impl {
            class HAZELCAST_API QueryResultSet : public impl::IdentifiedDataSerializableResponse {
            public:
                int getFactoryId() const;

                int getClassId() const;

                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > getResultData() const;

                void readData(serialization::ObjectDataInput& in);

            private:
                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > q;
            };
        }
    }
}


#endif //HAZELCAST_QueryResultSet

