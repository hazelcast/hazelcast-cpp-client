//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_MAP_CONTAINS_VALUE_R
#define HAZELCAST_MAP_CONTAINS_VALUE_R

#include "Portable.h"
#include "RetryableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace map {
            class ContainsValueRequest : public Portable, public RetryableRequest {
            public:
                ContainsValueRequest(const std::string& name, serialization::Data& value);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;


                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::Data& value;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
