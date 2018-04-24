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

#include "hazelcast/client/protocol/codec/CountDownLatchAwaitCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const CountDownLatchMessageType CountDownLatchAwaitCodec::REQUEST_TYPE = HZ_COUNTDOWNLATCH_AWAIT;
                const bool CountDownLatchAwaitCodec::RETRYABLE = false;
                const ResponseMessageConst CountDownLatchAwaitCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::auto_ptr<ClientMessage> CountDownLatchAwaitCodec::encodeRequest(
                        const std::string &name,
                        int64_t timeout) {
                    int32_t requiredDataSize = calculateDataSize(name, timeout);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) CountDownLatchAwaitCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(timeout);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t CountDownLatchAwaitCodec::calculateDataSize(
                        const std::string &name,
                        int64_t timeout) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(timeout);
                    return dataSize;
                }

                CountDownLatchAwaitCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (RESPONSE_TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException(
                                "CountDownLatchAwaitCodec::ResponseParameters::decode", clientMessage.getMessageType(),
                                RESPONSE_TYPE);
                    }


                    response = clientMessage.get<bool>();

                }

                CountDownLatchAwaitCodec::ResponseParameters
                CountDownLatchAwaitCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return CountDownLatchAwaitCodec::ResponseParameters(clientMessage);
                }

                CountDownLatchAwaitCodec::ResponseParameters::ResponseParameters(
                        const CountDownLatchAwaitCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }
            }
        }
    }
}

