//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SetCountRequest
#define HAZELCAST_SetCountRequest

#include "hazelcast/client/impl/ClientRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization{
            class PortableWriter;
        }

        namespace countdownlatch {
            class SetCountRequest : public impl::ClientRequest {
            public:
                SetCountRequest(const std::string& instanceName, int count);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                std::string instanceName;
                int count;
            };
        }
    }
}

#endif //HAZELCAST_SetCountRequest

