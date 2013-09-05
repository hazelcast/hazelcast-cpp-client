//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AddMessageListenerRequest
#define HAZELCAST_AddMessageListenerRequest

#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace topic {
            class AddMessageListenerRequest : public Portable {
            public:
                AddMessageListenerRequest(const std::string& instanceName);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::string instanceName;
            };
        }
    }
}


#endif //HAZELCAST_AddMessageListenerRequest
