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

#include "hazelcast/client/protocol/codec/TopicPublishCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TopicMessageType TopicPublishCodec::REQUEST_TYPE = HZ_TOPIC_PUBLISH;
                const bool TopicPublishCodec::RETRYABLE = false;
                const ResponseMessageConst TopicPublishCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::auto_ptr<ClientMessage> TopicPublishCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &message) {
                    int32_t requiredDataSize = calculateDataSize(name, message);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TopicPublishCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(message);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TopicPublishCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &message) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(message);
                    return dataSize;
                }


            }
        }
    }
}

