//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_REPLACE_REQUEST
#define HAZELCAST_MAP_REPLACE_REQUEST

#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace map {
            class ReplaceRequest : public Portable {
            public:
                ReplaceRequest(const std::string& name, serialization::Data& key, serialization::Data& value, int threadId);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::Data& key;
                serialization::Data& value;
                std::string name;
                int threadId;
            };
        }
    }
}

#endif //HAZELCAST_MAP_REMOVE_IF_SAME_REQUEST
