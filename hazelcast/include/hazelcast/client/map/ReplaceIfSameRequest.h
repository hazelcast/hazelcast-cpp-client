//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_REPLACE_IF_SAME_REQUEST
#define HAZELCAST_MAP_REPLACE_IF_SAME_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API ReplaceIfSameRequest : public impl::PortableRequest {
            public:
                ReplaceIfSameRequest(const std::string &name, serialization::pimpl::Data &key, serialization::pimpl::Data &testValue, serialization::pimpl::Data &value, long threadId);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::string name;
                serialization::pimpl::Data key;
                serialization::pimpl::Data value;
                serialization::pimpl::Data testValue;
                long threadId;
            };
        }
    }
}

#endif //HAZELCAST_MAP_REPLACE_IF_SAME_REQUEST

