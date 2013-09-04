//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_GET_ALL_REQUEST
#define HAZELCAST_MAP_GET_ALL_REQUEST

#include "Portable.h"
#include "RetryableRequest.h"
#include "Data.h"
#include <string>
#include <vector>

namespace hazelcast {
    namespace client {
        namespace map {
            class GetAllRequest : public Portable, public RetryableRequest {
            public:
                GetAllRequest(const std::string& name, std::vector<serialization::Data>& keys);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::vector<serialization::Data> keys;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_GET_ALL_REQUEST
