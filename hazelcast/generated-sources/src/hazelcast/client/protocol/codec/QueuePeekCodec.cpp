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

#include "hazelcast/client/protocol/codec/QueuePeekCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueuePeekCodec::REQUEST_TYPE = HZ_QUEUE_PEEK;
                const bool QueuePeekCodec::RETRYABLE = false;
                const ResponseMessageConst QueuePeekCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> QueuePeekCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueuePeekCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueuePeekCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                QueuePeekCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                QueuePeekCodec::ResponseParameters
                QueuePeekCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return QueuePeekCodec::ResponseParameters(clientMessage);
                }

                QueuePeekCodec::ResponseParameters::ResponseParameters(const QueuePeekCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

