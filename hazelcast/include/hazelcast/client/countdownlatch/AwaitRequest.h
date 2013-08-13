//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AwaitRequest
#define HAZELCAST_AwaitRequest

#include "../serialization/SerializationConstants.h"
#include "CountDownLatchPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace countdownlatch {
            class AwaitRequest : public Portable {
            public:
                AwaitRequest(const std::string& instanceName, long timeout);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:

                std::string instanceName;
                long timeout;
            };
        }
    }
}


#endif //HAZELCAST_AwaitRequest
