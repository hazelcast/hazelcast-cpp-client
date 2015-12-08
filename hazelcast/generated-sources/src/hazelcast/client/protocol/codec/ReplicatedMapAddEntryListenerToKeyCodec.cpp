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

#include "hazelcast/client/protocol/codec/ReplicatedMapAddEntryListenerToKeyCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapAddEntryListenerToKeyCodec::RequestParameters::TYPE = HZ_REPLICATEDMAP_ADDENTRYLISTENERTOKEY;
                const bool ReplicatedMapAddEntryListenerToKeyCodec::RequestParameters::RETRYABLE = false;
                const int32_t ReplicatedMapAddEntryListenerToKeyCodec::ResponseParameters::TYPE = 104;

                ReplicatedMapAddEntryListenerToKeyCodec::~ReplicatedMapAddEntryListenerToKeyCodec() {
                }

                std::auto_ptr<ClientMessage> ReplicatedMapAddEntryListenerToKeyCodec::RequestParameters::encode(
                        const std::string &name, 
                        const serialization::pimpl::Data &key, 
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, key, localOnly);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ReplicatedMapAddEntryListenerToKeyCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapAddEntryListenerToKeyCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const serialization::pimpl::Data &key, 
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                ReplicatedMapAddEntryListenerToKeyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ReplicatedMapAddEntryListenerToKeyCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<std::string >();
                }

                ReplicatedMapAddEntryListenerToKeyCodec::ResponseParameters ReplicatedMapAddEntryListenerToKeyCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ReplicatedMapAddEntryListenerToKeyCodec::ResponseParameters(clientMessage);
                }

                ReplicatedMapAddEntryListenerToKeyCodec::ResponseParameters::ResponseParameters(const ReplicatedMapAddEntryListenerToKeyCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }

                //************************ EVENTS START*************************************************************************//
                ReplicatedMapAddEntryListenerToKeyCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ReplicatedMapAddEntryListenerToKeyCodec::AbstractEventHandler::handle(std::auto_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ENTRY:
                        {
                            std::auto_ptr<serialization::pimpl::Data > key = clientMessage->getNullable<serialization::pimpl::Data >();

                            std::auto_ptr<serialization::pimpl::Data > value = clientMessage->getNullable<serialization::pimpl::Data >();

                            std::auto_ptr<serialization::pimpl::Data > oldValue = clientMessage->getNullable<serialization::pimpl::Data >();

                            std::auto_ptr<serialization::pimpl::Data > mergingValue = clientMessage->getNullable<serialization::pimpl::Data >();

                            int32_t eventType = clientMessage->get<int32_t >();
                            
                            std::string uuid = clientMessage->get<std::string >();
                            
                            int32_t numberOfAffectedEntries = clientMessage->get<int32_t >();
                            
                            handleEntry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                            break;
                        }
                        default:
                            char buf[300];
                            util::snprintf(buf, 300, "[ReplicatedMapAddEntryListenerToKeyCodec::AbstractEventHandler::handle] Unknown message type (%d) received on event handler.", clientMessage->getMessageType());
                            util::ILogger::getLogger().warning(buf);
                    }
                }
                //************************ EVENTS END **************************************************************************//

                ReplicatedMapAddEntryListenerToKeyCodec::ReplicatedMapAddEntryListenerToKeyCodec (const std::string &name, const serialization::pimpl::Data &key, const bool &localOnly)
                        : name_(name), key_(key), localOnly_(localOnly) {
                }

                //************************ IAddListenerCodec interface start ************************************************//
                std::auto_ptr<ClientMessage> ReplicatedMapAddEntryListenerToKeyCodec::encodeRequest() const {
                    return RequestParameters::encode(name_, key_, localOnly_);
                }

                std::string ReplicatedMapAddEntryListenerToKeyCodec::decodeResponse(ClientMessage &responseMessage) const {
                    return ResponseParameters::decode(responseMessage).response;
                }
                //************************ IAddListenerCodec interface ends *************************************************//

            }
        }
    }
}

