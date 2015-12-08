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

#include "hazelcast/client/protocol/codec/ClientAddDistributedObjectListenerCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientAddDistributedObjectListenerCodec::RequestParameters::TYPE = HZ_CLIENT_ADDDISTRIBUTEDOBJECTLISTENER;
                const bool ClientAddDistributedObjectListenerCodec::RequestParameters::RETRYABLE = false;
                const int32_t ClientAddDistributedObjectListenerCodec::ResponseParameters::TYPE = 104;

                ClientAddDistributedObjectListenerCodec::~ClientAddDistributedObjectListenerCodec() {
                }

                std::auto_ptr<ClientMessage> ClientAddDistributedObjectListenerCodec::RequestParameters::encode(
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(localOnly);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ClientAddDistributedObjectListenerCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientAddDistributedObjectListenerCodec::RequestParameters::calculateDataSize(
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                ClientAddDistributedObjectListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ClientAddDistributedObjectListenerCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<std::string >();
                }

                ClientAddDistributedObjectListenerCodec::ResponseParameters ClientAddDistributedObjectListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientAddDistributedObjectListenerCodec::ResponseParameters(clientMessage);
                }

                ClientAddDistributedObjectListenerCodec::ResponseParameters::ResponseParameters(const ClientAddDistributedObjectListenerCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }

                //************************ EVENTS START*************************************************************************//
                ClientAddDistributedObjectListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ClientAddDistributedObjectListenerCodec::AbstractEventHandler::handle(std::auto_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_DISTRIBUTEDOBJECT:
                        {
                            std::string name = clientMessage->get<std::string >();
                            
                            std::string serviceName = clientMessage->get<std::string >();
                            
                            std::string eventType = clientMessage->get<std::string >();
                            
                            handleDistributedObject(name, serviceName, eventType);
                            break;
                        }
                        default:
                            char buf[300];
                            util::snprintf(buf, 300, "[ClientAddDistributedObjectListenerCodec::AbstractEventHandler::handle] Unknown message type (%d) received on event handler.", clientMessage->getMessageType());
                            util::ILogger::getLogger().warning(buf);
                    }
                }
                //************************ EVENTS END **************************************************************************//

                ClientAddDistributedObjectListenerCodec::ClientAddDistributedObjectListenerCodec (const bool &localOnly)
                        : localOnly_(localOnly) {
                }

                //************************ IAddListenerCodec interface start ************************************************//
                std::auto_ptr<ClientMessage> ClientAddDistributedObjectListenerCodec::encodeRequest() const {
                    return RequestParameters::encode(localOnly_);
                }

                std::string ClientAddDistributedObjectListenerCodec::decodeResponse(ClientMessage &responseMessage) const {
                    return ResponseParameters::decode(responseMessage).response;
                }
                //************************ IAddListenerCodec interface ends *************************************************//

            }
        }
    }
}

