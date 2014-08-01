//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_GET_ALL_REQUEST
#define HAZELCAST_MAP_GET_ALL_REQUEST

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>
#include <vector>

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API GetAllRequest : public impl::ClientRequest {
            public:
                GetAllRequest(const std::string &name, const std::vector<serialization::pimpl::Data> &keys);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

                bool isRetryable() const;

            private:
                std::string name;
                std::vector<serialization::pimpl::Data> keys;
            };
        }
    }
}

#endif //HAZELCAST_MAP_GET_ALL_REQUEST

