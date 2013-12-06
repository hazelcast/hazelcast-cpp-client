//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_GET_REQUEST
#define HAZELCAST_MAP_GET_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/impl/RetryableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {

        namespace serialization {
            class Data;
        }
        namespace map {

            class HAZELCAST_API GetRequest : public impl::PortableRequest, public RetryableRequest {
            public:
                GetRequest(const std::string &name, serialization::Data &key);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

            private:
                serialization::Data &key;
                const std::string &name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_GET_REQUEST
