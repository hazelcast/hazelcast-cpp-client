//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_REMOVE_REQUEST
#define HAZELCAST_QUEUE_REMOVE_REQUEST

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl{
                class Data;
            }
        }
        namespace queue {
            class HAZELCAST_API RemoveRequest : public impl::ClientRequest {
            public:

                RemoveRequest(const std::string &name, serialization::pimpl::Data &data);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::string name;
                serialization::pimpl::Data data;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_REMOVE_REQUEST

