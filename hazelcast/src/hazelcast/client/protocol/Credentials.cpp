//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "Credentials.h"


namespace hazelcast {
    namespace client {
        namespace protocol {
            Credentials::Credentials() {

            };

            Credentials::Credentials(std::string principal, std::string password)
            : principal(principal) {
                char const *pasw = password.c_str();
                std::vector<byte> pwd(pasw, pasw + 8); //TODO
                this->password = pwd;
            };

            void Credentials::setPrincipal(std::string const & principal) {
                this->principal = principal;
            };

            void Credentials::setEndPoint(std::string const & endPoint) {
                this->endpoint = endPoint;
            };

            void Credentials::setPassword(std::string const & password) {
                char const *pasw = password.c_str();
                std::vector<byte> pwd(pasw, pasw + 8); //TODO
                this->password = pwd;
            };

            int Credentials::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;

            }

            int Credentials::getClassId() const {
                return protocol::SpiConstants::USERNAME_PWD_CRED;
            }


        }
    }
}
