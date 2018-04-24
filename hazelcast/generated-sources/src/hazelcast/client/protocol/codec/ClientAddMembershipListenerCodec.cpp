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

#include "hazelcast/util/Util.h"
#include "hazelcast/util/ILogger.h"

#include "hazelcast/client/protocol/codec/ClientAddMembershipListenerCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientAddMembershipListenerCodec::REQUEST_TYPE = HZ_CLIENT_ADDMEMBERSHIPLISTENER;
                const bool ClientAddMembershipListenerCodec::RETRYABLE = false;
                const ResponseMessageConst ClientAddMembershipListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::auto_ptr<ClientMessage> ClientAddMembershipListenerCodec::encodeRequest(
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(localOnly);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientAddMembershipListenerCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientAddMembershipListenerCodec::calculateDataSize(
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                ClientAddMembershipListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (RESPONSE_TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException(
                                "ClientAddMembershipListenerCodec::ResponseParameters::decode",
                                clientMessage.getMessageType(), RESPONSE_TYPE);
                    }


                    response = clientMessage.get<std::string>();

                }

                ClientAddMembershipListenerCodec::ResponseParameters
                ClientAddMembershipListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientAddMembershipListenerCodec::ResponseParameters(clientMessage);
                }

                ClientAddMembershipListenerCodec::ResponseParameters::ResponseParameters(
                        const ClientAddMembershipListenerCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }

                //************************ EVENTS START*************************************************************************//
                ClientAddMembershipListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ClientAddMembershipListenerCodec::AbstractEventHandler::handle(
                        std::auto_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_MEMBER: {
                            Member member = clientMessage->get<Member>();

                            int32_t eventType = clientMessage->get<int32_t>();


                            handleMemberEventV10(member, eventType);
                            break;
                        }
                        case protocol::EVENT_MEMBERLIST: {
                            std::vector<Member> members = clientMessage->getArray<Member>();


                            handleMemberListEventV10(members);
                            break;
                        }
                        case protocol::EVENT_MEMBERATTRIBUTECHANGE: {
                            std::string uuid = clientMessage->get<std::string>();

                            std::string key = clientMessage->get<std::string>();

                            int32_t operationType = clientMessage->get<int32_t>();

                            std::auto_ptr<std::string> value = clientMessage->getNullable<std::string>();


                            handleMemberAttributeChangeEventV10(uuid, key, operationType, value);
                            break;
                        }
                        default:
                            util::ILogger::getLogger().warning()
                                    << "[ClientAddMembershipListenerCodec::AbstractEventHandler::handle] Unknown message type ("
                                    << messageType << ") received on event handler.";
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

