//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_CREDENTIALS
#define HAZELCAST_CREDENTIALS

#include "ProtocolConstants.h"
#include "Portable.h"
#include <string>
#include <vector>

namespace hazelcast {
    namespace client {

        typedef unsigned char byte;

        namespace protocol {
            class Credentials : public Portable {
            public:
                Credentials();

                Credentials(std::string principal, std::string password);

                void setPrincipal(const std::string& principal);

                void setEndPoint(const std::string& endPoint);

                void setPassword(const std::string& password);

                int getFactoryId() const;

                int getClassId() const;


                template<typename HzWriter>
                inline void writePortable(HzWriter& writer) const{
                    writer.writeUTF("principal", principal);//dev
                    writer.writeUTF("endpoint", endpoint);//"
                    writer.writeByteArray("pwd", password);//dev-pass
                };

                template<typename HzReader>
                inline void readPortable(HzReader& reader) {
                    principal = reader.readUTF("principal");
                    endpoint = reader.readUTF("endpoint");
                    password = reader.readByteArray("pwd");
                };

            private:
                std::string principal;
                std::string endpoint;
                std::vector<byte> password;
            };
        }
    }
}


#endif //HAZELCAST_CREDENTIALS
