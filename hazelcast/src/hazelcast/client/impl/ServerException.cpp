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
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/impl/ServerException.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            ServerException::ServerException() {

            }

            ServerException::~ServerException() throw() {
            }

            char const *ServerException::what() const throw() {
                return (*details).c_str();
            }

            int ServerException::getFactoryId() const {
                return protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;
            }

            int ServerException::getClassId() const {
                return protocol::ProtocolConstants::HAZELCAST_SERVER_ERROR_ID;
            }

            void ServerException::readPortable(serialization::PortableReader &reader) {
                name = reader.readUTF("n");
                message = reader.readUTF("m");
                details = reader.readUTF("d");
                type = reader.readInt("t");
            }

        }
    }
}

