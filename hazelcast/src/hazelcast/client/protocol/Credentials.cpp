//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/protocol/Credentials.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"


namespace hazelcast {
    namespace client {
        namespace protocol {
            Credentials::Credentials(const std::string& principal, const std::string& password)
            : principal(principal) {
                char const *pasw = password.c_str();
                this->password.insert(this->password.begin(), pasw, pasw + password.size());
            };

            int Credentials::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;

            }

            int Credentials::getClassId() const {
                return protocol::SpiConstants::USERNAME_PWD_CRED;
            }

            void Credentials::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("principal", principal);//dev
                writer.writeUTF("endpoint", endpoint);//"
                writer.writeByteArray("pwd", password);//dev-pass
            };


        }
    }
}
