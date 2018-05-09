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

#include "hazelcast/client/protocol/codec/SemaphoreReducePermitsCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SemaphoreMessageType SemaphoreReducePermitsCodec::REQUEST_TYPE = HZ_SEMAPHORE_REDUCEPERMITS;
                const bool SemaphoreReducePermitsCodec::RETRYABLE = false;
                const ResponseMessageConst SemaphoreReducePermitsCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::auto_ptr<ClientMessage> SemaphoreReducePermitsCodec::encodeRequest(
                        const std::string &name,
                        int32_t reduction) {
                    int32_t requiredDataSize = calculateDataSize(name, reduction);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SemaphoreReducePermitsCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(reduction);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SemaphoreReducePermitsCodec::calculateDataSize(
                        const std::string &name,
                        int32_t reduction) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(reduction);
                    return dataSize;
                }

                SemaphoreReducePermitsCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                }

                SemaphoreReducePermitsCodec::ResponseParameters
                SemaphoreReducePermitsCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return SemaphoreReducePermitsCodec::ResponseParameters(clientMessage);
                }

                SemaphoreReducePermitsCodec::ResponseParameters::ResponseParameters(
                        const SemaphoreReducePermitsCodec::ResponseParameters &rhs) {
                }
            }
        }
    }
}

