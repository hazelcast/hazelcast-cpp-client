//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_CREDENTIALS
#define HAZELCAST_CREDENTIALS

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>
#include <vector>

namespace hazelcast {
    namespace client {

        typedef unsigned char byte;

        namespace protocol {
            class Credentials : public impl::PortableRequest {
            public:

                Credentials(const std::string &principal, const std::string &password);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

            private:
                std::string principal;
                std::string endpoint;
                std::vector<byte> password;
            };
        }
    }
}


#endif //HAZELCAST_CREDENTIALS
