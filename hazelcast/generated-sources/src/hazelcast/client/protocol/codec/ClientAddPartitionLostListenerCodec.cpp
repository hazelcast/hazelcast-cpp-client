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

#include "hazelcast/client/protocol/codec/ClientAddPartitionLostListenerCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientAddPartitionLostListenerCodec::RequestParameters::TYPE = HZ_CLIENT_ADDPARTITIONLOSTLISTENER;
                const bool ClientAddPartitionLostListenerCodec::RequestParameters::RETRYABLE = false;
                const int32_t ClientAddPartitionLostListenerCodec::ResponseParameters::TYPE = 104;

                ClientAddPartitionLostListenerCodec::~ClientAddPartitionLostListenerCodec() {
                }

                std::auto_ptr<ClientMessage> ClientAddPartitionLostListenerCodec::RequestParameters::encode(
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(localOnly);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ClientAddPartitionLostListenerCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientAddPartitionLostListenerCodec::RequestParameters::calculateDataSize(
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                ClientAddPartitionLostListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ClientAddPartitionLostListenerCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<std::string >();
                }

                ClientAddPartitionLostListenerCodec::ResponseParameters ClientAddPartitionLostListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientAddPartitionLostListenerCodec::ResponseParameters(clientMessage);
                }

                ClientAddPartitionLostListenerCodec::ResponseParameters::ResponseParameters(const ClientAddPartitionLostListenerCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }

                //************************ EVENTS START*************************************************************************//
                ClientAddPartitionLostListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ClientAddPartitionLostListenerCodec::AbstractEventHandler::handle(std::auto_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_PARTITIONLOST:
                        {
                            int32_t partitionId = clientMessage->get<int32_t >();
                            
                            int32_t lostBackupCount = clientMessage->get<int32_t >();
                            
                            std::auto_ptr<Address > source = clientMessage->getNullable<Address >();

                            handlePartitionLost(partitionId, lostBackupCount, source);
                            break;
                        }
                        default:
                            char buf[300];
                            util::snprintf(buf, 300, "[ClientAddPartitionLostListenerCodec::AbstractEventHandler::handle] Unknown message type (%d) received on event handler.", clientMessage->getMessageType());
                            util::ILogger::getLogger().warning(buf);
                    }
                }
                //************************ EVENTS END **************************************************************************//

                ClientAddPartitionLostListenerCodec::ClientAddPartitionLostListenerCodec (const bool &localOnly)
                        : localOnly_(localOnly) {
                }

                //************************ IAddListenerCodec interface start ************************************************//
                std::auto_ptr<ClientMessage> ClientAddPartitionLostListenerCodec::encodeRequest() const {
                    return RequestParameters::encode(localOnly_);
                }

                std::string ClientAddPartitionLostListenerCodec::decodeResponse(ClientMessage &responseMessage) const {
                    return ResponseParameters::decode(responseMessage).response;
                }
                //************************ IAddListenerCodec interface ends *************************************************//

            }
        }
    }
}

