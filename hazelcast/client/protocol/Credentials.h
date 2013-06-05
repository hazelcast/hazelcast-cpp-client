//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_CREDENTIALS
#define HAZELCAST_CREDENTIALS

#include "ProtocolConstants.h"
#include "../serialization/ConstantSerializers.h"
#include <string>
#include <vector>

namespace hazelcast {
    namespace client {

        typedef unsigned char byte;

        namespace protocol {
            class Credentials {
                template<typename HzWriter>
                friend void serialization::writePortable(HzWriter& writer, const hazelcast::client::protocol::Credentials& data);

                template<typename HzReader>
                friend void serialization::readPortable(HzReader& reader, hazelcast::client::protocol::Credentials& data);
                
            public:
                Credentials();

                Credentials(std::string principal, std::string password);

                void setPrincipal(const std::string& principal);

                void setEndPoint(const std::string& endPoint);

                void setPassword(const std::string& password);

            private:
                std::string principal;
                std::string endpoint;
                std::vector<byte> password;
            };
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace serialization {

            inline int getFactoryId(const protocol::Credentials& ar) {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;
            };

            inline int getClassId(const protocol::Credentials& ar) {
                return protocol::SpiConstants::CREDENTIALS_ID;
            };


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const ::hazelcast::client::protocol::Credentials& data) {
                writer["principal"] << data.principal; //dev
                writer["endpoint"] << data.endpoint; //""
                writer["pwd"] << data.password; //dev-pass


            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, ::hazelcast::client::protocol::Credentials& data) {
                reader["principal"] >> data.principal; //dev
                reader["endpoint"] >> data.endpoint; //""
                reader["pwd"] >> data.password; //dev-pass
            };

        }
    }
}

#endif //HAZELCAST_CREDENTIALS
