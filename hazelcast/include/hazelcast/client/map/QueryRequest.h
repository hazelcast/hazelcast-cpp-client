//
// Created by sancar koyunlu on 6/19/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_QUERY_REQUEST
#define HAZELCAST_QUERY_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/impl/RetryableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API QueryRequest : public impl::PortableRequest, public RetryableRequest {
            public:
                QueryRequest(const std::string &name, const std::string &iterationType, const std::string &sql);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

            private:
                const std::string& name;
                const std::string& iterationType;
                const std::string& sql;
            };
        }
    }
}

#endif //HAZELCAST_QUERY_REQUEST
