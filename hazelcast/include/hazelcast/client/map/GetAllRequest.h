//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_GET_ALL_REQUEST
#define HAZELCAST_MAP_GET_ALL_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/impl/RetryableRequest.h"
#include "hazelcast/client/serialization/Data.h"
#include <string>
#include <vector>

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API GetAllRequest : public impl::PortableRequest, public RetryableRequest {
            public:
                GetAllRequest(const std::string& name,const std::vector<serialization::Data>& keys);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                const std::vector<serialization::Data> &keys;
                const std::string& name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_GET_ALL_REQUEST
