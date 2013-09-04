//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_REMOVE_REQUEST
#define HAZELCAST_QUEUE_REMOVE_REQUEST

#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class RemoveRequest : public Portable {
            public:

                RemoveRequest(const std::string& name, serialization::Data& data);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;


                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::Data& data;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_REMOVE_REQUEST
