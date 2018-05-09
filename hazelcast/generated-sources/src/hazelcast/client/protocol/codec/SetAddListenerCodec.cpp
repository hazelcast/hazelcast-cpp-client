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

#include "hazelcast/client/protocol/codec/SetAddListenerCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetAddListenerCodec::REQUEST_TYPE = HZ_SET_ADDLISTENER;
                const bool SetAddListenerCodec::RETRYABLE = false;
                const ResponseMessageConst SetAddListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::auto_ptr<ClientMessage> SetAddListenerCodec::encodeRequest(
                        const std::string &name,
                        bool includeValue,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, includeValue, localOnly);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SetAddListenerCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(includeValue);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetAddListenerCodec::calculateDataSize(
                        const std::string &name,
                        bool includeValue,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(includeValue);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                SetAddListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                SetAddListenerCodec::ResponseParameters
                SetAddListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return SetAddListenerCodec::ResponseParameters(clientMessage);
                }

                SetAddListenerCodec::ResponseParameters::ResponseParameters(
                        const SetAddListenerCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }

                //************************ EVENTS START*************************************************************************//
                SetAddListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void SetAddListenerCodec::AbstractEventHandler::handle(
                        std::auto_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ITEM: {
                            std::auto_ptr<serialization::pimpl::Data> item = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::string uuid = clientMessage->get<std::string>();

                            int32_t eventType = clientMessage->get<int32_t>();


                            handleItemEventV10(item, uuid, eventType);
                            break;
                        }
                        default:
                            util::ILogger::getLogger().warning()
                                    << "[SetAddListenerCodec::AbstractEventHandler::handle] Unknown message type ("
                                    << messageType << ") received on event handler.";
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

