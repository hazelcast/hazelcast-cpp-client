//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_SET_REQUEST
#define HAZELCAST_MAP_SET_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace map {
            class HAZELCAST_API SetRequest : public impl::PortableRequest {
            public:
                SetRequest(const std::string &name, serialization::Data &key, serialization::Data &value, long threadId, long ttl);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;


            private:
                serialization::Data &key;
                serialization::Data &value;
                std::string name;
                long threadId;
                long ttl;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
