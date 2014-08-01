//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetCountRequest
#define HAZELCAST_GetCountRequest

#include "hazelcast/client/impl/ClientRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class PortableWriter;
        }
        namespace countdownlatch {
            class HAZELCAST_API GetCountRequest : public impl::ClientRequest {
            public:
                GetCountRequest(const std::string &instanceName);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;


                bool isRetryable() const;

            private:
                std::string instanceName;
            };
        }
    }
}

#endif //HAZELCAST_GetCountRequest

