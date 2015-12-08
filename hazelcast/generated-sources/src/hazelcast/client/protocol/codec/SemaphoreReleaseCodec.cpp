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



#include "hazelcast/client/protocol/codec/SemaphoreReleaseCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SemaphoreMessageType SemaphoreReleaseCodec::RequestParameters::TYPE = HZ_SEMAPHORE_RELEASE;
                const bool SemaphoreReleaseCodec::RequestParameters::RETRYABLE = false;
                const int32_t SemaphoreReleaseCodec::ResponseParameters::TYPE = 100;
                std::auto_ptr<ClientMessage> SemaphoreReleaseCodec::RequestParameters::encode(
                        const std::string &name, 
                        int32_t permits) {
                    int32_t requiredDataSize = calculateDataSize(name, permits);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)SemaphoreReleaseCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(permits);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SemaphoreReleaseCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        int32_t permits) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(permits);
                    return dataSize;
                }

                SemaphoreReleaseCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("SemaphoreReleaseCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }
                }

                SemaphoreReleaseCodec::ResponseParameters SemaphoreReleaseCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return SemaphoreReleaseCodec::ResponseParameters(clientMessage);
                }

                SemaphoreReleaseCodec::ResponseParameters::ResponseParameters(const SemaphoreReleaseCodec::ResponseParameters &rhs) {
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

