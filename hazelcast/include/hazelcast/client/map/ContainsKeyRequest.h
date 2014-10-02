//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_CONTAINS_KEY_R
#define HAZELCAST_MAP_CONTAINS_KEY_R

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class ContainsKeyRequest : public impl::ClientRequest {
            public:
                ContainsKeyRequest(const std::string& name, const serialization::pimpl::Data& key, long threadId);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

                bool isRetryable() const;

            private:
                std::string name;
                serialization::pimpl::Data key;
                long threadId;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST

