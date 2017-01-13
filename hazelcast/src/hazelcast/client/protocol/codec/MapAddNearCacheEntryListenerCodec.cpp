/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/protocol/codec/MapAddNearCacheEntryListenerCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapAddNearCacheEntryListenerCodec::RequestParameters::TYPE = HZ_MAP_ADDNEARCACHEENTRYLISTENER;
                const bool MapAddNearCacheEntryListenerCodec::RequestParameters::RETRYABLE = false;
                const int32_t MapAddNearCacheEntryListenerCodec::ResponseParameters::TYPE = 104;

                MapAddNearCacheEntryListenerCodec::~MapAddNearCacheEntryListenerCodec() {
                }

                std::auto_ptr<ClientMessage> MapAddNearCacheEntryListenerCodec::RequestParameters::encode(
                        const std::string &name, 
                        int32_t listenerFlags, 
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, listenerFlags, localOnly);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)MapAddNearCacheEntryListenerCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(listenerFlags);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapAddNearCacheEntryListenerCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        int32_t listenerFlags, 
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(listenerFlags);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                MapAddNearCacheEntryListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("MapAddNearCacheEntryListenerCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<std::string >();
                }

                MapAddNearCacheEntryListenerCodec::ResponseParameters MapAddNearCacheEntryListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapAddNearCacheEntryListenerCodec::ResponseParameters(clientMessage);
                }

                MapAddNearCacheEntryListenerCodec::ResponseParameters::ResponseParameters(const MapAddNearCacheEntryListenerCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }

                //************************ EVENTS START*************************************************************************//
                MapAddNearCacheEntryListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void MapAddNearCacheEntryListenerCodec::AbstractEventHandler::handle(std::auto_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_IMAPINVALIDATION:
                        {
                            std::auto_ptr<serialization::pimpl::Data > key = clientMessage->getNullable<serialization::pimpl::Data >();

                            handleIMapInvalidation(key);
                            break;
                        }
                        case protocol::EVENT_IMAPBATCHINVALIDATION:
                        {
                            std::vector<serialization::pimpl::Data > keys = clientMessage->getArray<serialization::pimpl::Data >();
                            
                            handleIMapBatchInvalidation(keys);
                            break;
                        }
                        default:
                            char buf[300];
                            util::snprintf(buf, 300, "[MapAddNearCacheEntryListenerCodec::AbstractEventHandler::handle] Unknown message type (%d) received on event handler.", clientMessage->getMessageType());
                            util::ILogger::getLogger().warning(buf);
                    }
                }
                //************************ EVENTS END **************************************************************************//

                MapAddNearCacheEntryListenerCodec::MapAddNearCacheEntryListenerCodec (const std::string &name, const int32_t &listenerFlags, const bool &localOnly)
                        : name_(name), listenerFlags_(listenerFlags), localOnly_(localOnly) {
                }

                //************************ IAddListenerCodec interface start ************************************************//
                std::auto_ptr<ClientMessage> MapAddNearCacheEntryListenerCodec::encodeRequest() const {
                    return RequestParameters::encode(name_, listenerFlags_, localOnly_);
                }

                std::string MapAddNearCacheEntryListenerCodec::decodeResponse(ClientMessage &responseMessage) const {
                    return ResponseParameters::decode(responseMessage).response;
                }
                //************************ IAddListenerCodec interface ends *************************************************//

            }
        }
    }
}

