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


#include "hazelcast/client/protocol/AuthenticationRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"

namespace hazelcast {
    namespace client {
        namespace protocol {

            AuthenticationRequest::AuthenticationRequest(const Credentials &credentials)
            :credentials(credentials)
            , firstConnection(true) {

            }

            void AuthenticationRequest::setPrincipal(Principal *principal) {
                this->principal = principal;
            }

            void AuthenticationRequest::setFirstConnection(bool firstConnection) {
                this->firstConnection = firstConnection;
            }


            int AuthenticationRequest::getFactoryId() const {
                return protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;
            }

            int AuthenticationRequest::getClassId() const {
                return protocol::ProtocolConstants::AUTHENTICATION_REQUEST_ID;
            }

            void AuthenticationRequest::write(serialization::PortableWriter &writer) const {
                writer.writePortable("credentials", &credentials);
                if (principal == NULL) {
                    writer.writeNullPortable<Principal>("principal");
                } else {
                    writer.writePortable("principal", principal);
                }
                writer.writeBoolean("firstConnection", firstConnection);
            }
        }
    }
}
