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

#include "hazelcast/client/protocol/codec/MapAddEntryListenerToKeyCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapAddEntryListenerToKeyCodec::REQUEST_TYPE = HZ_MAP_ADDENTRYLISTENERTOKEY;
                const bool MapAddEntryListenerToKeyCodec::RETRYABLE = false;
                const ResponseMessageConst MapAddEntryListenerToKeyCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::auto_ptr<ClientMessage> MapAddEntryListenerToKeyCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        bool includeValue,
                        int32_t listenerFlags,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, key, includeValue, listenerFlags, localOnly);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapAddEntryListenerToKeyCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(includeValue);
                    clientMessage->set(listenerFlags);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapAddEntryListenerToKeyCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        bool includeValue,
                        int32_t listenerFlags,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(includeValue);
                    dataSize += ClientMessage::calculateDataSize(listenerFlags);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                MapAddEntryListenerToKeyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (RESPONSE_TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException(
                                "MapAddEntryListenerToKeyCodec::ResponseParameters::decode",
                                clientMessage.getMessageType(), RESPONSE_TYPE);
                    }


                    response = clientMessage.get<std::string>();

                }

                MapAddEntryListenerToKeyCodec::ResponseParameters
                MapAddEntryListenerToKeyCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapAddEntryListenerToKeyCodec::ResponseParameters(clientMessage);
                }

                MapAddEntryListenerToKeyCodec::ResponseParameters::ResponseParameters(
                        const MapAddEntryListenerToKeyCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }

                //************************ EVENTS START*************************************************************************//
                MapAddEntryListenerToKeyCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void MapAddEntryListenerToKeyCodec::AbstractEventHandler::handle(
                        std::auto_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ENTRY: {
                            std::auto_ptr<serialization::pimpl::Data> key = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::auto_ptr<serialization::pimpl::Data> value = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::auto_ptr<serialization::pimpl::Data> oldValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::auto_ptr<serialization::pimpl::Data> mergingValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            int32_t eventType = clientMessage->get<int32_t>();

                            std::string uuid = clientMessage->get<std::string>();

                            int32_t numberOfAffectedEntries = clientMessage->get<int32_t>();


                            handleEntryEventV10(key, value, oldValue, mergingValue, eventType, uuid,
                                                numberOfAffectedEntries);
                            break;
                        }
                        default:
                            util::ILogger::getLogger().warning()
                                    << "[MapAddEntryListenerToKeyCodec::AbstractEventHandler::handle] Unknown message type ("
                                    << messageType << ") received on event handler.";
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

