//
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_PRINCIPAL
#define HAZELCAST_PRINCIPAL

#include "ProtocolConstants.h"
#include "../serialization/SerializationConstants.h"
#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace protocol {
            class Principal : public Portable {
            public:
                Principal();

                Principal(std::string uuid, std::string ownerUuid);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::string uuid;
                std::string ownerUuid;
            };
        }
    }
}
#endif //HAZELCAST_PRINCIPAL
