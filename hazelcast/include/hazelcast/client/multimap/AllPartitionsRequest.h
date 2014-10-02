//
// Created by sancar koyunlu on 9/10/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_AllPartitionsRequest
#define HAZELCAST_AllPartitionsRequest

#include "hazelcast/client/impl/ClientRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace multimap {
            class AllPartitionsRequest : public impl::ClientRequest {
            public:
                AllPartitionsRequest(const std::string &name);

                int getFactoryId() const;

                virtual void write(serialization::PortableWriter &writer) const;

            private:
                std::string name;

            };
        }
    }
}

#endif //HAZELCAST_AllPartitionsRequest

