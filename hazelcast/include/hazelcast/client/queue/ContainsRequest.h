//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_CONTAINS_REQUEST
#define HAZELCAST_QUEUE_CONTAINS_REQUEST

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/impl/ClientRequest.h"
#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class ContainsRequest : public impl::ClientRequest {
            public:

                ContainsRequest(const std::string &name, const std::vector<serialization::pimpl::Data> &dataList);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

                bool isRetryable() const;

            private:
                std::string name;
                std::vector<serialization::pimpl::Data> dataList;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_CONTAINS_REQUEST

