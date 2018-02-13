/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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



#include "hazelcast/client/protocol/codec/ClientAuthenticationCustomCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/Address.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientAuthenticationCustomCodec::RequestParameters::TYPE = HZ_CLIENT_AUTHENTICATIONCUSTOM;
                const bool ClientAuthenticationCustomCodec::RequestParameters::RETRYABLE = true;
                const int32_t ClientAuthenticationCustomCodec::ResponseParameters::TYPE = 107;
                std::auto_ptr<ClientMessage> ClientAuthenticationCustomCodec::RequestParameters::encode(
                        const serialization::pimpl::Data &credentials, 
                        const std::string *uuid, 
                        const std::string *ownerUuid, 
                        bool isOwnerConnection, 
                        const std::string &clientType, 
                        uint8_t serializationVersion) {
                    int32_t requiredDataSize = calculateDataSize(credentials, uuid, ownerUuid, isOwnerConnection, clientType, serializationVersion);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ClientAuthenticationCustomCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(credentials);
                    clientMessage->set(uuid);
                    clientMessage->set(ownerUuid);
                    clientMessage->set(isOwnerConnection);
                    clientMessage->set(clientType);
                    clientMessage->set(serializationVersion);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientAuthenticationCustomCodec::RequestParameters::calculateDataSize(
                        const serialization::pimpl::Data &credentials, 
                        const std::string *uuid, 
                        const std::string *ownerUuid, 
                        bool isOwnerConnection, 
                        const std::string &clientType, 
                        uint8_t serializationVersion) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(credentials);
                    dataSize += ClientMessage::calculateDataSize(uuid);
                    dataSize += ClientMessage::calculateDataSize(ownerUuid);
                    dataSize += ClientMessage::calculateDataSize(isOwnerConnection);
                    dataSize += ClientMessage::calculateDataSize(clientType);
                    dataSize += ClientMessage::calculateDataSize(serializationVersion);
                    return dataSize;
                }

                ClientAuthenticationCustomCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ClientAuthenticationCustomCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    status = clientMessage.get<uint8_t >();

                    address = clientMessage.getNullable<Address >();

                    uuid = clientMessage.getNullable<std::string >();

                    ownerUuid = clientMessage.getNullable<std::string >();

                    serializationVersion = clientMessage.get<uint8_t >();
                }

                ClientAuthenticationCustomCodec::ResponseParameters ClientAuthenticationCustomCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientAuthenticationCustomCodec::ResponseParameters(clientMessage);
                }

                ClientAuthenticationCustomCodec::ResponseParameters::ResponseParameters(const ClientAuthenticationCustomCodec::ResponseParameters &rhs) {
                        status = rhs.status;
                        address = std::auto_ptr<Address>(new Address(*rhs.address));
                        uuid = std::auto_ptr<std::string>(new std::string(*rhs.uuid));
                        ownerUuid = std::auto_ptr<std::string>(new std::string(*rhs.ownerUuid));
                        serializationVersion = rhs.serializationVersion;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

