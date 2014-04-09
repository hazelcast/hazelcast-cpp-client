//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/protocol/UsernamePasswordCredentials.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"


namespace hazelcast {
    namespace client {
        namespace protocol {
            UsernamePasswordCredentials::UsernamePasswordCredentials(const std::string &principal, const std::string &password)
            : principal(principal) {
                char const *pasw = password.c_str();
                this->password.insert(this->password.begin(), pasw, pasw + password.size());
            };

            int UsernamePasswordCredentials::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;

            }

            int UsernamePasswordCredentials::getClassId() const {
                return protocol::SpiConstants::USERNAME_PWD_CRED;
            }

            void UsernamePasswordCredentials::writePortable(serialization::PortableWriter &writer) const {
                writer.writeUTF("principal", principal);//dev
                writer.writeUTF("endpoint", endpoint);//"
                writer.writeByteArray("pwd", password);//dev-pass
            };

            void UsernamePasswordCredentials::readPortable(serialization::PortableReader &reader) {
            };

            void UsernamePasswordCredentials::setEndpoint(const std::string &endpoint) {
                this->endpoint = endpoint;
            }

            const std::string &UsernamePasswordCredentials::getEndpoint() const {
                return endpoint;
            }

            const std::string &UsernamePasswordCredentials::getPrincipal() const {
                return principal;
            }
        }
    }
}

