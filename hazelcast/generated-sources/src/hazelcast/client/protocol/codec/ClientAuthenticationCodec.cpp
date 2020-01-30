/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/util/Util.h"
#include "hazelcast/util/ILogger.h"

#include "hazelcast/client/protocol/codec/ClientAuthenticationCodec.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/Member.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientAuthenticationCodec::REQUEST_TYPE = HZ_CLIENT_AUTHENTICATION;
                const bool ClientAuthenticationCodec::RETRYABLE = true;
                const ResponseMessageConst ClientAuthenticationCodec::RESPONSE_TYPE = (ResponseMessageConst) 107;

                std::unique_ptr<ClientMessage> ClientAuthenticationCodec::encodeRequest(
                        const std::string &username,
                        const std::string &password,
                        const std::string *uuid,
                        const std::string *ownerUuid,
                        bool isOwnerConnection,
                        const std::string &clientType,
                        uint8_t serializationVersion,
                        const std::string &clientHazelcastVersion) {
                    int32_t requiredDataSize = calculateDataSize(username, password, uuid, ownerUuid, isOwnerConnection,
                                                                 clientType, serializationVersion,
                                                                 clientHazelcastVersion);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientAuthenticationCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(username);
                    clientMessage->set(password);
                    clientMessage->set(uuid);
                    clientMessage->set(ownerUuid);
                    clientMessage->set(isOwnerConnection);
                    clientMessage->set(clientType);
                    clientMessage->set(serializationVersion);
                    clientMessage->set(clientHazelcastVersion);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientAuthenticationCodec::calculateDataSize(
                        const std::string &username,
                        const std::string &password,
                        const std::string *uuid,
                        const std::string *ownerUuid,
                        bool isOwnerConnection,
                        const std::string &clientType,
                        uint8_t serializationVersion,
                        const std::string &clientHazelcastVersion) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(username);
                    dataSize += ClientMessage::calculateDataSize(password);
                    dataSize += ClientMessage::calculateDataSize(uuid);
                    dataSize += ClientMessage::calculateDataSize(ownerUuid);
                    dataSize += ClientMessage::calculateDataSize(isOwnerConnection);
                    dataSize += ClientMessage::calculateDataSize(clientType);
                    dataSize += ClientMessage::calculateDataSize(serializationVersion);
                    dataSize += ClientMessage::calculateDataSize(clientHazelcastVersion);
                    return dataSize;
                }

                ClientAuthenticationCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    serverHazelcastVersionExist = false;
                    clientUnregisteredMembersExist = false;


                    status = clientMessage.get<uint8_t>();


                    address = clientMessage.getNullable<Address>();


                    uuid = clientMessage.getNullable<std::string>();


                    ownerUuid = clientMessage.getNullable<std::string>();


                    serializationVersion = clientMessage.get<uint8_t>();
                    if (clientMessage.isComplete()) {
                        return;
                    }

                    serverHazelcastVersion = clientMessage.get<std::string>();
                    serverHazelcastVersionExist = true;

                    clientUnregisteredMembers = clientMessage.getNullableArray<Member>();
                    clientUnregisteredMembersExist = true;
                }

                ClientAuthenticationCodec::ResponseParameters
                ClientAuthenticationCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientAuthenticationCodec::ResponseParameters(clientMessage);
                }

                ClientAuthenticationCodec::ResponseParameters::ResponseParameters(
                        const ClientAuthenticationCodec::ResponseParameters &rhs) {
                    status = rhs.status;
                    address = std::unique_ptr<Address>(new Address(*rhs.address));
                    uuid = std::unique_ptr<std::string>(new std::string(*rhs.uuid));
                    ownerUuid = std::unique_ptr<std::string>(new std::string(*rhs.ownerUuid));
                    serializationVersion = rhs.serializationVersion;
                    serverHazelcastVersion = rhs.serverHazelcastVersion;
                    clientUnregisteredMembers = std::unique_ptr<std::vector<Member> >(
                            new std::vector<Member>(*rhs.clientUnregisteredMembers));
                }

            }
        }
    }
}

