//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_ITERATOR_REQUEST
#define HAZELCAST_QUEUE_ITERATOR_REQUEST

#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class IteratorRequest : public Portable {
            public:
                IteratorRequest(const std::string& name);

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

#endif //HAZELCAST_QUEUE_CLEAR_REQUEST
