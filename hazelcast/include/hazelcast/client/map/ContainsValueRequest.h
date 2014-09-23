//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_MAP_CONTAINS_VALUE_R
#define HAZELCAST_MAP_CONTAINS_VALUE_R

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API ContainsValueRequest : public impl::ClientRequest {
            public:
                ContainsValueRequest(const std::string &name, const serialization::pimpl::Data &value);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                const std::string name;
                const serialization::pimpl::Data value;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST

