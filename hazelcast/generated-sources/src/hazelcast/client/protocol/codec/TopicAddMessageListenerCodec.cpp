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

#include "hazelcast/client/protocol/codec/TopicAddMessageListenerCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TopicMessageType TopicAddMessageListenerCodec::REQUEST_TYPE = HZ_TOPIC_ADDMESSAGELISTENER;
                const bool TopicAddMessageListenerCodec::RETRYABLE = false;
                const ResponseMessageConst TopicAddMessageListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> TopicAddMessageListenerCodec::encodeRequest(
                        const std::string &name,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TopicAddMessageListenerCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TopicAddMessageListenerCodec::calculateDataSize(
                        const std::string &name,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                TopicAddMessageListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                TopicAddMessageListenerCodec::ResponseParameters
                TopicAddMessageListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return TopicAddMessageListenerCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                TopicAddMessageListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void TopicAddMessageListenerCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_TOPIC: {
                            serialization::pimpl::Data item = clientMessage->get<serialization::pimpl::Data>();

                            int64_t publishTime = clientMessage->get<int64_t>();

                            std::string uuid = clientMessage->get<std::string>();


                            handleTopicEventV10(item, publishTime, uuid);
                            break;
                        }
                        default:
                            getLogger()->warning()
                                    << "[TopicAddMessageListenerCodec::AbstractEventHandler::handle] Unknown message type ("
                                    << messageType << ") received on event handler.";
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

