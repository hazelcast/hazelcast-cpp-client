//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetCountRequest
#define HAZELCAST_GetCountRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/impl/RetryableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization{
            class PortableWriter;
        }
        namespace countdownlatch {
            class HAZELCAST_API GetCountRequest : public impl::PortableRequest, public RetryableRequest {
            public:
                GetCountRequest(const std::string& instanceName);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;


            private:

                std::string instanceName;
            };
        }
    }
}

#endif //HAZELCAST_GetCountRequest
