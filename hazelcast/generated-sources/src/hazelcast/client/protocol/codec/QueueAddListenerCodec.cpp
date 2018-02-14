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

#include "hazelcast/client/protocol/codec/QueueAddListenerCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueAddListenerCodec::RequestParameters::TYPE = HZ_QUEUE_ADDLISTENER;
                const bool QueueAddListenerCodec::RequestParameters::RETRYABLE = false;
                const int32_t QueueAddListenerCodec::ResponseParameters::TYPE = 104;

                QueueAddListenerCodec::~QueueAddListenerCodec() {
                }

                std::auto_ptr<ClientMessage> QueueAddListenerCodec::RequestParameters::encode(
                        const std::string &name, 
                        bool includeValue, 
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, includeValue, localOnly);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)QueueAddListenerCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(includeValue);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueAddListenerCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        bool includeValue, 
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(includeValue);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                QueueAddListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("QueueAddListenerCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<std::string >();
                }

                QueueAddListenerCodec::ResponseParameters QueueAddListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return QueueAddListenerCodec::ResponseParameters(clientMessage);
                }

                QueueAddListenerCodec::ResponseParameters::ResponseParameters(const QueueAddListenerCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }

                //************************ EVENTS START*************************************************************************//
                QueueAddListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void QueueAddListenerCodec::AbstractEventHandler::handle(std::auto_ptr<protocol::ClientMessage> clientMessage) {
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
                            util::hz_snprintf(buf, 300,
                                              "[QueueAddListenerCodec::AbstractEventHandler::handle] Unknown message type (%d) received on event handler.",
                                              clientMessage->getMessageType());
                            util::ILogger::getLogger().warning(buf);
                    }
                }
                //************************ EVENTS END **************************************************************************//

                QueueAddListenerCodec::QueueAddListenerCodec (const std::string &name, const bool &includeValue, const bool &localOnly)
                        : name_(name), includeValue_(includeValue), localOnly_(localOnly) {
                }

                //************************ IAddListenerCodec interface start ************************************************//
                std::auto_ptr<ClientMessage> QueueAddListenerCodec::encodeRequest() const {
                    return RequestParameters::encode(name_, includeValue_, localOnly_);
                }

                std::string QueueAddListenerCodec::decodeResponse(ClientMessage &responseMessage) const {
                    return ResponseParameters::decode(responseMessage).response;
                }
                //************************ IAddListenerCodec interface ends *************************************************//

            }
        }
    }
}

