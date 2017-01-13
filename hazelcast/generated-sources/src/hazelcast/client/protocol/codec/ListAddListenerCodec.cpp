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

#include "hazelcast/client/protocol/codec/ListAddListenerCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListAddListenerCodec::RequestParameters::TYPE = HZ_LIST_ADDLISTENER;
                const bool ListAddListenerCodec::RequestParameters::RETRYABLE = false;
                const int32_t ListAddListenerCodec::ResponseParameters::TYPE = 104;

                ListAddListenerCodec::~ListAddListenerCodec() {
                }

                std::auto_ptr<ClientMessage> ListAddListenerCodec::RequestParameters::encode(
                        const std::string &name, 
                        bool includeValue, 
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, includeValue, localOnly);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ListAddListenerCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(includeValue);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListAddListenerCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        bool includeValue, 
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(includeValue);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                ListAddListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ListAddListenerCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<std::string >();
                }

                ListAddListenerCodec::ResponseParameters ListAddListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ListAddListenerCodec::ResponseParameters(clientMessage);
                }

                ListAddListenerCodec::ResponseParameters::ResponseParameters(const ListAddListenerCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }

                //************************ EVENTS START*************************************************************************//
                ListAddListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ListAddListenerCodec::AbstractEventHandler::handle(std::auto_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ITEM:
                        {
                            std::auto_ptr<serialization::pimpl::Data > item = clientMessage->getNullable<serialization::pimpl::Data >();

                            std::string uuid = clientMessage->get<std::string >();
                            
                            int32_t eventType = clientMessage->get<int32_t >();
                            
                            handleItem(item, uuid, eventType);
                            break;
                        }
                        default:
                            char buf[300];
                            util::snprintf(buf, 300, "[ListAddListenerCodec::AbstractEventHandler::handle] Unknown message type (%d) received on event handler.", clientMessage->getMessageType());
                            util::ILogger::getLogger().warning(buf);
                    }
                }
                //************************ EVENTS END **************************************************************************//

                ListAddListenerCodec::ListAddListenerCodec (const std::string &name, const bool &includeValue, const bool &localOnly)
                        : name_(name), includeValue_(includeValue), localOnly_(localOnly) {
                }

                //************************ IAddListenerCodec interface start ************************************************//
                std::auto_ptr<ClientMessage> ListAddListenerCodec::encodeRequest() const {
                    return RequestParameters::encode(name_, includeValue_, localOnly_);
                }

                std::string ListAddListenerCodec::decodeResponse(ClientMessage &responseMessage) const {
                    return ResponseParameters::decode(responseMessage).response;
                }
                //************************ IAddListenerCodec interface ends *************************************************//

            }
        }
    }
}

