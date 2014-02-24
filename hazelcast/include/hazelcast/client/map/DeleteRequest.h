//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_DELETE_REQUEST
#define HAZELCAST_MAP_DELETE_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API DeleteRequest : public impl::PortableRequest {
            public:
                DeleteRequest(const std::string &name, serialization::pimpl::Data &key, long threadId);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                serialization::pimpl::Data key;
                std::string name;
                long threadId;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
