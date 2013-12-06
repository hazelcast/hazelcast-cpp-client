//
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_PRINCIPAL
#define HAZELCAST_PRINCIPAL

#include "hazelcast/client/serialization/SerializationConstants.h"
#include "hazelcast/client/Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace protocol {
            class HAZELCAST_API Principal : public Portable {
            public:
                Principal();

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::string uuid;
                std::string ownerUuid;
            };
        }
    }
}
#endif //HAZELCAST_PRINCIPAL
