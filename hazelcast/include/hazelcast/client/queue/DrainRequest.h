//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_MAX_SIZE_REQUEST
#define HAZELCAST_QUEUE_MAX_SIZE_REQUEST

#include "hazelcast/client/Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class DrainRequest : public Portable {
            public:
                DrainRequest(const std::string& name, int maxSize);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::string name;
                int maxSize;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_ADD_LISTENER_REQUEST
