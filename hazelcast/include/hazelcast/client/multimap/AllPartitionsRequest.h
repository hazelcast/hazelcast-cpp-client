//
// Created by sancar koyunlu on 9/10/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_AllPartitionsRequest
#define HAZELCAST_AllPartitionsRequest

#include "Request.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace multimap {
            class AllPartitionsRequest : public impl::Request {
            public:
                AllPartitionsRequest(const std::string& name);

                int getFactoryId() const;

                virtual void writePortable(serialization::PortableWriter& writer) const;

            private:
                std::string name;

            };
        }
    }
}

#endif //HAZELCAST_AllPartitionsRequest
