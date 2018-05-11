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

#include "hazelcast/client/protocol/codec/MapAddNearCacheEntryListenerCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/util/UUID.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapAddNearCacheEntryListenerCodec::REQUEST_TYPE = HZ_MAP_ADDNEARCACHEENTRYLISTENER;
                const bool MapAddNearCacheEntryListenerCodec::RETRYABLE = false;
                const ResponseMessageConst MapAddNearCacheEntryListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::auto_ptr<ClientMessage> MapAddNearCacheEntryListenerCodec::encodeRequest(
                        const std::string &name,
                        int32_t listenerFlags,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, listenerFlags, localOnly);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapAddNearCacheEntryListenerCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(listenerFlags);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapAddNearCacheEntryListenerCodec::calculateDataSize(
                        const std::string &name,
                        int32_t listenerFlags,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(listenerFlags);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                MapAddNearCacheEntryListenerCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                MapAddNearCacheEntryListenerCodec::ResponseParameters
                MapAddNearCacheEntryListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapAddNearCacheEntryListenerCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                MapAddNearCacheEntryListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void MapAddNearCacheEntryListenerCodec::AbstractEventHandler::handle(
                        std::auto_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_IMAPINVALIDATION: {
                            std::auto_ptr<serialization::pimpl::Data> key = clientMessage->getNullable<serialization::pimpl::Data>();

                            if (clientMessage->isComplete()) {
                                handleIMapInvalidationEventV10(key);
                                return;
                            }
                            std::string sourceUuid = clientMessage->get<std::string>();

                            util::UUID partitionUuid = clientMessage->get<util::UUID>();

                            int64_t sequence = clientMessage->get<int64_t>();


                            handleIMapInvalidationEventV14(key, sourceUuid, partitionUuid, sequence);
                            break;
                        }
                        case protocol::EVENT_IMAPBATCHINVALIDATION: {
                            std::vector<serialization::pimpl::Data> keys = clientMessage->getArray<serialization::pimpl::Data>();

                            if (clientMessage->isComplete()) {
                                handleIMapBatchInvalidationEventV10(keys);
                                return;
                            }
                            std::vector<std::string> sourceUuids = clientMessage->getArray<std::string>();

                            std::vector<util::UUID> partitionUuids = clientMessage->getArray<util::UUID>();

                            std::vector<int64_t> sequences = clientMessage->getArray<int64_t>();


                            handleIMapBatchInvalidationEventV14(keys, sourceUuids, partitionUuids, sequences);
                            break;
                        }
                        default:
                            util::ILogger::getLogger().warning()
                                    << "[MapAddNearCacheEntryListenerCodec::AbstractEventHandler::handle] Unknown message type ("
                                    << messageType << ") received on event handler.";
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

