/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
            }

            int UsernamePasswordCredentials::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;

            }

            int UsernamePasswordCredentials::getClassId() const {
                return protocol::SpiConstants::USERNAME_PWD_CRED;
            }

            void UsernamePasswordCredentials::writePortable(serialization::PortableWriter &writer) const {
                writer.writeUTF("principal", &principal);//dev
                writer.writeUTF("endpoint", &endpoint);//"
                writer.writeByteArray("pwd", &password);//dev-pass
            }

            void UsernamePasswordCredentials::readPortable(serialization::PortableReader &reader) {
            }

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

