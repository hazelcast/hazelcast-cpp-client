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

#include "hazelcast/client/protocol/codec/ReplicatedMapAddNearCacheEntryListenerCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapAddNearCacheEntryListenerCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_ADDNEARCACHEENTRYLISTENER;
                const bool ReplicatedMapAddNearCacheEntryListenerCodec::RETRYABLE = false;
                const ResponseMessageConst ReplicatedMapAddNearCacheEntryListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> ReplicatedMapAddNearCacheEntryListenerCodec::encodeRequest(
                        const std::string &name, 
                        bool includeValue, 
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, includeValue, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ReplicatedMapAddNearCacheEntryListenerCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(includeValue);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapAddNearCacheEntryListenerCodec::calculateDataSize(
                        const std::string &name, 
                        bool includeValue, 
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(includeValue);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                    ReplicatedMapAddNearCacheEntryListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                        
                        

                    response = clientMessage.get<std::string >();
                            
                    }

                    ReplicatedMapAddNearCacheEntryListenerCodec::ResponseParameters ReplicatedMapAddNearCacheEntryListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                        return ReplicatedMapAddNearCacheEntryListenerCodec::ResponseParameters(clientMessage);
                    }



                //************************ EVENTS START*************************************************************************//
                ReplicatedMapAddNearCacheEntryListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ReplicatedMapAddNearCacheEntryListenerCodec::AbstractEventHandler::handle(std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ENTRY:
                        {
                            std::unique_ptr<serialization::pimpl::Data > key = clientMessage->getNullable<serialization::pimpl::Data >();

                            std::unique_ptr<serialization::pimpl::Data > value = clientMessage->getNullable<serialization::pimpl::Data >();

                            std::unique_ptr<serialization::pimpl::Data > oldValue = clientMessage->getNullable<serialization::pimpl::Data >();

                            std::unique_ptr<serialization::pimpl::Data > mergingValue = clientMessage->getNullable<serialization::pimpl::Data >();

                            int32_t eventType = clientMessage->get<int32_t >();
                            
                            std::string uuid = clientMessage->get<std::string >();
                            
                            int32_t numberOfAffectedEntries = clientMessage->get<int32_t >();
                            

                                handleEntryEventV10(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                                break;
                        }
                        default:
                            getLogger()->warning("[ReplicatedMapAddNearCacheEntryListenerCodec::AbstractEventHandler::handle] Unknown message type (", messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

