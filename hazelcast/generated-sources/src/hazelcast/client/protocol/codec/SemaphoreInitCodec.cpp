/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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



#include "hazelcast/client/protocol/codec/SemaphoreInitCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SemaphoreMessageType SemaphoreInitCodec::RequestParameters::TYPE = HZ_SEMAPHORE_INIT;
                const bool SemaphoreInitCodec::RequestParameters::RETRYABLE = false;
                const int32_t SemaphoreInitCodec::ResponseParameters::TYPE = 101;
                std::auto_ptr<ClientMessage> SemaphoreInitCodec::RequestParameters::encode(
                        const std::string &name, 
                        int32_t permits) {
                    int32_t requiredDataSize = calculateDataSize(name, permits);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)SemaphoreInitCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(permits);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SemaphoreInitCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        int32_t permits) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(permits);
                    return dataSize;
                }

                SemaphoreInitCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("SemaphoreInitCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<bool >();
                }

                SemaphoreInitCodec::ResponseParameters SemaphoreInitCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return SemaphoreInitCodec::ResponseParameters(clientMessage);
                }

                SemaphoreInitCodec::ResponseParameters::ResponseParameters(const SemaphoreInitCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

