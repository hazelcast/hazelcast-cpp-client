//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/protocol/Credentials.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "PortableReader.h"


namespace hazelcast {
    namespace client {
        namespace protocol {
            Credentials::Credentials() {

            };

            Credentials::Credentials(std::string& principal, std::string& password)
            : principal(principal) {
                char const *pasw = password.c_str();
                this->password.insert(this->password.begin(), pasw, pasw + password.size());
            };

            void Credentials::setPrincipal(std::string const & principal) {
                this->principal = principal;
            };

            void Credentials::setEndPoint(std::string const & endPoint) {
                this->endpoint = endPoint;
            };

            void Credentials::setPassword(std::string const & password) {
                char const *pasw = password.c_str();
                this->password.insert(this->password.begin(), pasw, pasw + password.size());
            };

            int Credentials::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;

            }

            int Credentials::getClassId() const {
                return protocol::SpiConstants::USERNAME_PWD_CRED;
            }

            void Credentials::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("principal", principal);//dev
                writer.writeUTF("endpoint", endpoint);//"
                writer.writeByteArray("pwd", password);//dev-pass
            };


            void Credentials::readPortable(serialization::PortableReader& reader) {
                principal = reader.readUTF("principal");
                endpoint = reader.readUTF("endpoint");
                password = reader.readByteArray("pwd");
            };


        }
    }
}
