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



#include "hazelcast/client/protocol/codec/CountDownLatchTrySetCountCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const CountDownLatchMessageType CountDownLatchTrySetCountCodec::RequestParameters::TYPE = HZ_COUNTDOWNLATCH_TRYSETCOUNT;
                const bool CountDownLatchTrySetCountCodec::RequestParameters::RETRYABLE = false;
                const int32_t CountDownLatchTrySetCountCodec::ResponseParameters::TYPE = 101;
                std::auto_ptr<ClientMessage> CountDownLatchTrySetCountCodec::RequestParameters::encode(
                        const std::string &name, 
                        int32_t count) {
                    int32_t requiredDataSize = calculateDataSize(name, count);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)CountDownLatchTrySetCountCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(count);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t CountDownLatchTrySetCountCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        int32_t count) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(count);
                    return dataSize;
                }

                CountDownLatchTrySetCountCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("CountDownLatchTrySetCountCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<bool >();
                }

                CountDownLatchTrySetCountCodec::ResponseParameters CountDownLatchTrySetCountCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return CountDownLatchTrySetCountCodec::ResponseParameters(clientMessage);
                }

                CountDownLatchTrySetCountCodec::ResponseParameters::ResponseParameters(const CountDownLatchTrySetCountCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

