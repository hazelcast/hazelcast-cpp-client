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

#include "hazelcast/client/protocol/codec/SemaphoreAvailablePermitsCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SemaphoreMessageType SemaphoreAvailablePermitsCodec::REQUEST_TYPE = HZ_SEMAPHORE_AVAILABLEPERMITS;
                const bool SemaphoreAvailablePermitsCodec::RETRYABLE = true;
                const ResponseMessageConst SemaphoreAvailablePermitsCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> SemaphoreAvailablePermitsCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SemaphoreAvailablePermitsCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SemaphoreAvailablePermitsCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                SemaphoreAvailablePermitsCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                SemaphoreAvailablePermitsCodec::ResponseParameters
                SemaphoreAvailablePermitsCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return SemaphoreAvailablePermitsCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

