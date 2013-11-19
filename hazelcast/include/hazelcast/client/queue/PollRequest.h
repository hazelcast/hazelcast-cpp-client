//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_POLL_REQUEST
#define HAZELCAST_QUEUE_POLL_REQUEST

#include "hazelcast/client/Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class PollRequest : public Portable {
            public:
                PollRequest(const std::string& name, long timeout);

                PollRequest(const std::string& name);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;


                void readPortable(serialization::PortableReader& reader);

            private:
                std::string name;
                long timeoutInMillis;
            };
        }
    }
}

#endif //HAZELCAST_POLL_REQUEST
