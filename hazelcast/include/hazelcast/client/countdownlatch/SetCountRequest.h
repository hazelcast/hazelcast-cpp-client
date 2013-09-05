//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SetCountRequest
#define HAZELCAST_SetCountRequest

#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace countdownlatch {
            class SetCountRequest : public Portable {
            public:
                SetCountRequest(const std::string& instanceName, int count);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::string instanceName;
                int count;
            };
        }
    }
}

#endif //HAZELCAST_SetCountRequest
