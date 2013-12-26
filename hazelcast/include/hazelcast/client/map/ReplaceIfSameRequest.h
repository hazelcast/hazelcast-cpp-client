//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_REPLACE_IF_SAME_REQUEST
#define HAZELCAST_MAP_REPLACE_IF_SAME_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace map {
            class HAZELCAST_API ReplaceIfSameRequest : public impl::PortableRequest {
            public:
                ReplaceIfSameRequest(const std::string& name, serialization::Data& key, serialization::Data& testValue, serialization::Data& value, int threadId);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;



            private:
                serialization::Data& key;
                serialization::Data& value;
                serialization::Data& testValue;
                std::string name;
                int threadId;
            };
        }
    }
}

#endif //HAZELCAST_MAP_REPLACE_IF_SAME_REQUEST
