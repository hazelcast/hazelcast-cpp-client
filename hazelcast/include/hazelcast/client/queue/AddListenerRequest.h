//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_ADD_LISTENER_REQUEST
#define HAZELCAST_QUEUE_ADD_LISTENER_REQUEST

#include "hazelcast/client/impl/ClientRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class HAZELCAST_API AddListenerRequest : public impl::ClientRequest {
            public:
                AddListenerRequest(const std::string& name, bool includeValue);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                std::string name;
                bool includeValue;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_ADD_LISTENER_REQUEST

