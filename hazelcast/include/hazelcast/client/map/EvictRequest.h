//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MAP_EVICT_REQUEST
#define HAZELCAST_MAP_EVICT_REQUEST

#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace map {
            class EvictRequest : public Portable {
            public:
                EvictRequest(const std::string& name, serialization::Data& key, int threadId);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::Data& key;
                std::string name;
                int threadId;
            };
        }
    }
}

#endif //HAZELCAST_MAP_EVICT_REQUEST
