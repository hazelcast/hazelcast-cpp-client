//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_GET_REQUEST
#define HAZELCAST_MAP_GET_REQUEST

#include "../serialization/Data.h"
#include "PortableHook.h"
#include "Portable.h"

namespace hazelcast {
    namespace client {

        namespace serialization {
            class Data;
        }
        namespace map {

            class GetRequest : public Portable ,public RetryableRequest{
            public:
                GetRequest(std::string& name, serialization::Data& key);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::Data& key;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_GET_REQUEST
