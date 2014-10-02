//
// Created by sancar koyunlu on 6/19/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_QUERY_REQUEST
#define HAZELCAST_QUERY_REQUEST

#include "hazelcast/client/impl/ClientRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class QueryRequest : public impl::ClientRequest {
            public:
                QueryRequest(const std::string &name, const std::string &iterationType, const std::string &sql);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

                bool isRetryable() const;

            private:
                std::string name;
                std::string iterationType;
                std::string sql;
            };
        }
    }
}

#endif //HAZELCAST_QUERY_REQUEST

