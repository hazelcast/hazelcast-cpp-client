//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AtomicLongRequest
#define HAZELCAST_AtomicLongRequest

#include "../serialization/SerializationConstants.h"
#include "AtomicLongPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            class AtomicLongRequest : public Portable {
            public:
                AtomicLongRequest(const std::string& instanceName, long delta);

                virtual int getFactoryId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                long delta;
                std::string instanceName;
            };
        }
    }
}

#endif //HAZELCAST_AtomicLongRequest
