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
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            bool ClientResponse::isException() const {
                return exception;
            }

            serialization::pimpl::Data const &ClientResponse::getData() const {
                return data;
            }

            int ClientResponse::getCallId() const {
                return callId;
            }

            int ClientResponse::getFactoryId() const {
                return protocol::ProtocolConstants::CLIENT_DS_FACTORY;
            }

            int ClientResponse::getClassId() const {
                return protocol::ProtocolConstants::CLIENT_RESPONSE;
            }

            void ClientResponse::readData(serialization::ObjectDataInput & in) {
                callId = in.readInt();
                exception = in.readBoolean();
                data = in.readData();
            }
        }
    }
}
