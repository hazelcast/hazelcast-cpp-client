//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_GET_ENTRY_VIEW_REQUEST
#define HAZELCAST_MAP_GET_ENTRY_VIEW_REQUEST

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class GetEntryViewRequest : public impl::ClientRequest {
            public:
                GetEntryViewRequest(const std::string &name, const serialization::pimpl::Data &key);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

                bool isRetryable() const;

            private:
                std::string name;
                serialization::pimpl::Data key;
            };
        }
    }
}

#endif //HAZELCAST_MAP_GET_ENTRY_VIEW_REQUEST

