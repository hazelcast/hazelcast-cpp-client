//
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_PRINCIPAL
#define HAZELCAST_PRINCIPAL

#include "hazelcast/client/serialization/SerializationConstants.h"
#include "hazelcast/client/impl/PortableResponse.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace protocol {
            class Principal : public impl::PortableResponse {
            public:
                Principal();

                int getFactoryId() const;

                int getClassId() const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::string uuid;
                std::string ownerUuid;
            };
        }
    }
}
#endif //HAZELCAST_PRINCIPAL
