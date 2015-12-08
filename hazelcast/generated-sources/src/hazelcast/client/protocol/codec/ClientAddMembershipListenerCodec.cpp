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
                const ClientMessageType ClientAddMembershipListenerCodec::RequestParameters::TYPE = HZ_CLIENT_ADDMEMBERSHIPLISTENER;
                const bool ClientAddMembershipListenerCodec::RequestParameters::RETRYABLE = false;
                const int32_t ClientAddMembershipListenerCodec::ResponseParameters::TYPE = 104;

                ClientAddMembershipListenerCodec::~ClientAddMembershipListenerCodec() {
                }

                std::auto_ptr<ClientMessage> ClientAddMembershipListenerCodec::RequestParameters::encode(
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(localOnly);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ClientAddMembershipListenerCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientAddMembershipListenerCodec::RequestParameters::calculateDataSize(
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                ClientAddMembershipListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ClientAddMembershipListenerCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<std::string >();
                }

                ClientAddMembershipListenerCodec::ResponseParameters ClientAddMembershipListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientAddMembershipListenerCodec::ResponseParameters(clientMessage);
                }

                ClientAddMembershipListenerCodec::ResponseParameters::ResponseParameters(const ClientAddMembershipListenerCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }

                //************************ EVENTS START*************************************************************************//
                ClientAddMembershipListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ClientAddMembershipListenerCodec::AbstractEventHandler::handle(std::auto_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_MEMBER:
                        {
                            Member member = clientMessage->get<Member >();
                            
                            int32_t eventType = clientMessage->get<int32_t >();
                            
                            handleMember(member, eventType);
                            break;
                        }
                        case protocol::EVENT_MEMBERLIST:
                        {
                            std::vector<Member > members = clientMessage->getArray<Member >();
                            
                            handleMemberList(members);
                            break;
                        }
                        case protocol::EVENT_MEMBERATTRIBUTECHANGE:
                        {
                            std::string uuid = clientMessage->get<std::string >();
                            
                            std::string key = clientMessage->get<std::string >();
                            
                            int32_t operationType = clientMessage->get<int32_t >();
                            
                            std::auto_ptr<std::string > value = clientMessage->getNullable<std::string >();

                            handleMemberAttributeChange(uuid, key, operationType, value);
                            break;
                        }
                        default:
                            char buf[300];
                            util::snprintf(buf, 300, "[ClientAddMembershipListenerCodec::AbstractEventHandler::handle] Unknown message type (%d) received on event handler.", clientMessage->getMessageType());
                            util::ILogger::getLogger().warning(buf);
                    }
                }
                //************************ EVENTS END **************************************************************************//

                ClientAddMembershipListenerCodec::ClientAddMembershipListenerCodec (const bool &localOnly)
                        : localOnly_(localOnly) {
                }

                //************************ IAddListenerCodec interface start ************************************************//
                std::auto_ptr<ClientMessage> ClientAddMembershipListenerCodec::encodeRequest() const {
                    return RequestParameters::encode(localOnly_);
                }

                std::string ClientAddMembershipListenerCodec::decodeResponse(ClientMessage &responseMessage) const {
                    return ResponseParameters::decode(responseMessage).response;
                }
                //************************ IAddListenerCodec interface ends *************************************************//

            }
        }
    }
}

