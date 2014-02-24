//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_GET_REQUEST
#define HAZELCAST_MAP_GET_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {

        namespace serialization {
            namespace pimpl{
                class Data;
            }

        }
        namespace map {

            class HAZELCAST_API GetRequest : public impl::PortableRequest {
            public:
                GetRequest(const std::string &name, serialization::pimpl::Data &key);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

                bool isRetryable() const;

            private:
                serialization::pimpl::Data key;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_GET_REQUEST
