//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_CONTAINS_REQUEST
#define HAZELCAST_QUEUE_CONTAINS_REQUEST

#include "../serialization/Data.h"
#include "Portable.h"
#include "RetryableRequest.h"
#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class ContainsRequest : public Portable, public RetryableRequest {
            public:

                ContainsRequest(const std::string& name, std::vector<serialization::Data>& dataList);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;


                void readPortable(serialization::PortableReader& reader);

            private:
                std::vector<serialization::Data>& dataList;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_CONTAINS_REQUEST
