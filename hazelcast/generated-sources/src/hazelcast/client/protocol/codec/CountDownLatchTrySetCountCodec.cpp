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

#include "hazelcast/client/protocol/codec/CountDownLatchTrySetCountCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const CountDownLatchMessageType CountDownLatchTrySetCountCodec::REQUEST_TYPE = HZ_COUNTDOWNLATCH_TRYSETCOUNT;
                const bool CountDownLatchTrySetCountCodec::RETRYABLE = false;
                const ResponseMessageConst CountDownLatchTrySetCountCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::auto_ptr<ClientMessage> CountDownLatchTrySetCountCodec::encodeRequest(
                        const std::string &name,
                        int32_t count) {
                    int32_t requiredDataSize = calculateDataSize(name, count);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) CountDownLatchTrySetCountCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(count);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t CountDownLatchTrySetCountCodec::calculateDataSize(
                        const std::string &name,
                        int32_t count) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(count);
                    return dataSize;
                }

                CountDownLatchTrySetCountCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (RESPONSE_TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException(
                                "CountDownLatchTrySetCountCodec::ResponseParameters::decode",
                                clientMessage.getMessageType(), RESPONSE_TYPE);
                    }


                    response = clientMessage.get<bool>();

                }

                CountDownLatchTrySetCountCodec::ResponseParameters
                CountDownLatchTrySetCountCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return CountDownLatchTrySetCountCodec::ResponseParameters(clientMessage);
                }

                CountDownLatchTrySetCountCodec::ResponseParameters::ResponseParameters(
                        const CountDownLatchTrySetCountCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }
            }
        }
    }
}

