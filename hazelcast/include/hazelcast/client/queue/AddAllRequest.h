//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_ADD_ALL_REQUEST
#define HAZELCAST_QUEUE_ADD_ALL_REQUEST

#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class HAZELCAST_API AddAllRequest : public impl::PortableRequest {
            public:

                AddAllRequest(const std::string &name, std::vector<serialization::Data> &dataList);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::vector<serialization::Data> &dataList;
                const std::string &name;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_ADD_ALL_REQUEST
