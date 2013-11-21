//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_REMOVE_REQUEST
#define HAZELCAST_QUEUE_REMOVE_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace queue {
            class RemoveRequest : public impl::PortableRequest {
            public:

                RemoveRequest(const std::string& name, serialization::Data& data);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                serialization::Data& data;
                const std::string& name;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_REMOVE_REQUEST
