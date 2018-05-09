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

#include "hazelcast/client/protocol/codec/SemaphoreInitCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SemaphoreMessageType SemaphoreInitCodec::REQUEST_TYPE = HZ_SEMAPHORE_INIT;
                const bool SemaphoreInitCodec::RETRYABLE = false;
                const ResponseMessageConst SemaphoreInitCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::auto_ptr<ClientMessage> SemaphoreInitCodec::encodeRequest(
                        const std::string &name,
                        int32_t permits) {
                    int32_t requiredDataSize = calculateDataSize(name, permits);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SemaphoreInitCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(permits);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SemaphoreInitCodec::calculateDataSize(
                        const std::string &name,
                        int32_t permits) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(permits);
                    return dataSize;
                }

                SemaphoreInitCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                SemaphoreInitCodec::ResponseParameters
                SemaphoreInitCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return SemaphoreInitCodec::ResponseParameters(clientMessage);
                }

                SemaphoreInitCodec::ResponseParameters::ResponseParameters(
                        const SemaphoreInitCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }
            }
        }
    }
}

