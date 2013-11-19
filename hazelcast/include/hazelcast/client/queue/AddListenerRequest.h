//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_ADD_LISTENER_REQUEST
#define HAZELCAST_QUEUE_ADD_LISTENER_REQUEST

#include "hazelcast/client/Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class AddListenerRequest : public Portable {
            public:
                AddListenerRequest(const std::string& name, bool includeValue);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::string name;
                bool includeValue;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_ADD_LISTENER_REQUEST
