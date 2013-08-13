//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_PUT_REQUEST
#define HAZELCAST_MAP_PUT_REQUEST

#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {

        namespace serialization {
            class Data;
        }

        namespace map {
            class PutRequest : public Portable {
            public:
                PutRequest(const std::string& name, serialization::Data& key, serialization::Data& value, int threadId, long ttl);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::Data& key;
                serialization::Data& value;
                std::string name;
                int threadId;
                long ttl;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
