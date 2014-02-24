//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_COMPARE_AND_REMOVE_REQUEST
#define HAZELCAST_QUEUE_COMPARE_AND_REMOVE_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class HAZELCAST_API CompareAndRemoveRequest : public impl::PortableRequest {
            public:

                CompareAndRemoveRequest(const std::string &name, std::vector<serialization::pimpl::Data> &dataList, bool retain);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::vector<serialization::pimpl::Data> dataList;
                std::string name;
                bool retain;
            };
        }
    }
}

#endif //HAZELCAST_OFFER_REQUEST
