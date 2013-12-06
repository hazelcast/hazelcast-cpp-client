//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_GET_ENTRY_VIEW_REQUEST
#define HAZELCAST_MAP_GET_ENTRY_VIEW_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/impl/RetryableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace map {
            class HAZELCAST_API GetEntryViewRequest : public impl::PortableRequest, public RetryableRequest {
            public:
                GetEntryViewRequest(const std::string &name, const serialization::Data &key);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

            private:
                const serialization::Data &key;
                const std::string &name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_GET_ENTRY_VIEW_REQUEST
