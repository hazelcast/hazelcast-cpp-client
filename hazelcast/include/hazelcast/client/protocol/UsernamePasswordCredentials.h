//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_CREDENTIALS
#define HAZELCAST_CREDENTIALS


#include "hazelcast/client/Credentials.h"
#include <string>
#include <vector>

namespace hazelcast {
    namespace client {
        namespace protocol {
            /**
             * Credentials is a container object for endpoint (Members and Clients)
             * security attributes.
             *
             * It is used on authentication process by javax.security.auth.spi.LoginModules.
             */
            class HAZELCAST_API UsernamePasswordCredentials : public Credentials {
            public:

                UsernamePasswordCredentials(const std::string &principal, const std::string &password);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

                void readPortable(serialization::PortableReader &reader);

                const std::string &getEndpoint() const;

                void setEndpoint(const std::string &endpoint);

                const std::string &getPrincipal() const;

            private:
                std::string principal;

                std::string endpoint;
                std::vector<byte> password;
            };
        }
    }
}


#endif //HAZELCAST_CREDENTIALS

