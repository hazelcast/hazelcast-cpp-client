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

#include "hazelcast/client/protocol/codec/MapAddPartitionLostListenerCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapAddPartitionLostListenerCodec::RequestParameters::TYPE = HZ_MAP_ADDPARTITIONLOSTLISTENER;
                const bool MapAddPartitionLostListenerCodec::RequestParameters::RETRYABLE = false;
                const int32_t MapAddPartitionLostListenerCodec::ResponseParameters::TYPE = 104;

                MapAddPartitionLostListenerCodec::~MapAddPartitionLostListenerCodec() {
                }

                std::auto_ptr<ClientMessage> MapAddPartitionLostListenerCodec::RequestParameters::encode(
                        const std::string &name, 
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, localOnly);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)MapAddPartitionLostListenerCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapAddPartitionLostListenerCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                MapAddPartitionLostListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("MapAddPartitionLostListenerCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<std::string >();
                }

                MapAddPartitionLostListenerCodec::ResponseParameters MapAddPartitionLostListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapAddPartitionLostListenerCodec::ResponseParameters(clientMessage);
                }

                MapAddPartitionLostListenerCodec::ResponseParameters::ResponseParameters(const MapAddPartitionLostListenerCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }

                //************************ EVENTS START*************************************************************************//
                MapAddPartitionLostListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void MapAddPartitionLostListenerCodec::AbstractEventHandler::handle(std::auto_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_MAPPARTITIONLOST:
                        {
                            int32_t partitionId = clientMessage->get<int32_t >();
                            
                            std::string uuid = clientMessage->get<std::string >();
                            
                            handleMapPartitionLost(partitionId, uuid);
                            break;
                        }
                        default:
                            char buf[300];
                            util::snprintf(buf, 300, "[MapAddPartitionLostListenerCodec::AbstractEventHandler::handle] Unknown message type (%d) received on event handler.", clientMessage->getMessageType());
                            util::ILogger::getLogger().warning(buf);
                    }
                }
                //************************ EVENTS END **************************************************************************//

                MapAddPartitionLostListenerCodec::MapAddPartitionLostListenerCodec (const std::string &name, const bool &localOnly)
                        : name_(name), localOnly_(localOnly) {
                }

                //************************ IAddListenerCodec interface start ************************************************//
                std::auto_ptr<ClientMessage> MapAddPartitionLostListenerCodec::encodeRequest() const {
                    return RequestParameters::encode(name_, localOnly_);
                }

                std::string MapAddPartitionLostListenerCodec::decodeResponse(ClientMessage &responseMessage) const {
                    return ResponseParameters::decode(responseMessage).response;
                }
                //************************ IAddListenerCodec interface ends *************************************************//

            }
        }
    }
}

