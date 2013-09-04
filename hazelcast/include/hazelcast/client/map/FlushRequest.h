//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_MAP_FLUSH_REQUEST
#define HAZELCAST_MAP_FLUSH_REQUEST

#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class FlushRequest : public Portable {
            public:
                FlushRequest(const std::string& name);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::string name;
            };
        }
    }
}


#endif //MAP_FLUSH_REQUEST
