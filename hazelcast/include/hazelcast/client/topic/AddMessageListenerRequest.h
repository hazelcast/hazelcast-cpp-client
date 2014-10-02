//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AddMessageListenerRequest
#define HAZELCAST_AddMessageListenerRequest

#include "hazelcast/client/impl/ClientRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace topic {
            class AddMessageListenerRequest : public impl::ClientRequest {
            public:
                AddMessageListenerRequest(const std::string& instanceName);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                std::string instanceName;
            };
        }
    }
}


#endif //HAZELCAST_AddMessageListenerRequest

