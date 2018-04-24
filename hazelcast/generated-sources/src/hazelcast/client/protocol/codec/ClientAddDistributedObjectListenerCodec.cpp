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

#include "hazelcast/client/protocol/codec/ClientAddDistributedObjectListenerCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientAddDistributedObjectListenerCodec::REQUEST_TYPE = HZ_CLIENT_ADDDISTRIBUTEDOBJECTLISTENER;
                const bool ClientAddDistributedObjectListenerCodec::RETRYABLE = false;
                const ResponseMessageConst ClientAddDistributedObjectListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::auto_ptr<ClientMessage> ClientAddDistributedObjectListenerCodec::encodeRequest(
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(localOnly);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientAddDistributedObjectListenerCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientAddDistributedObjectListenerCodec::calculateDataSize(
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                ClientAddDistributedObjectListenerCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {
                    if (RESPONSE_TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException(
                                "ClientAddDistributedObjectListenerCodec::ResponseParameters::decode",
                                clientMessage.getMessageType(), RESPONSE_TYPE);
                    }


                    response = clientMessage.get<std::string>();

                }

                ClientAddDistributedObjectListenerCodec::ResponseParameters
                ClientAddDistributedObjectListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientAddDistributedObjectListenerCodec::ResponseParameters(clientMessage);
                }

                ClientAddDistributedObjectListenerCodec::ResponseParameters::ResponseParameters(
                        const ClientAddDistributedObjectListenerCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }

                //************************ EVENTS START*************************************************************************//
                ClientAddDistributedObjectListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ClientAddDistributedObjectListenerCodec::AbstractEventHandler::handle(
                        std::auto_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_DISTRIBUTEDOBJECT: {
                            std::string name = clientMessage->get<std::string>();

                            std::string serviceName = clientMessage->get<std::string>();

                            std::string eventType = clientMessage->get<std::string>();


                            handleDistributedObjectEventV10(name, serviceName, eventType);
                            break;
                        }
                        default:
                            util::ILogger::getLogger().warning()
                                    << "[ClientAddDistributedObjectListenerCodec::AbstractEventHandler::handle] Unknown message type ("
                                    << messageType << ") received on event handler.";
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

