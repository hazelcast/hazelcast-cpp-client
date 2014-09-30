//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_MAP_IS_LOCKED_REQUEST
#define HAZELCAST_MAP_IS_LOCKED_REQUEST

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class IsLockedRequest : public impl::ClientRequest {
            public:
                IsLockedRequest(const std::string& name, const serialization::pimpl::Data& key);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                std::string name;
                serialization::pimpl::Data key;
            };
        }
    }
}


#endif //MAP_CLEAR_REQUEST

