//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_CREDENTIALS
#define HAZELCAST_CREDENTIALS

#include "ProtocolConstants.h"
#include "hazelcast/client/Portable.h"
#include <string>
#include <vector>

namespace hazelcast {
    namespace client {

        typedef unsigned char byte;

        namespace protocol {
            class Credentials : public Portable {
            public:
                Credentials();

                Credentials(std::string& principal, std::string& password);

                void setPrincipal(const std::string& principal);

                void setEndPoint(const std::string& endPoint);

                void setPassword(const std::string& password);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::string principal;
                std::string endpoint;
                std::vector<byte> password;
            };
        }
    }
}


#endif //HAZELCAST_CREDENTIALS
