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



#include "hazelcast/client/protocol/codec/SemaphoreReducePermitsCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SemaphoreMessageType SemaphoreReducePermitsCodec::RequestParameters::TYPE = HZ_SEMAPHORE_REDUCEPERMITS;
                const bool SemaphoreReducePermitsCodec::RequestParameters::RETRYABLE = false;
                const int32_t SemaphoreReducePermitsCodec::ResponseParameters::TYPE = 100;
                std::auto_ptr<ClientMessage> SemaphoreReducePermitsCodec::RequestParameters::encode(
                        const std::string &name, 
                        int32_t reduction) {
                    int32_t requiredDataSize = calculateDataSize(name, reduction);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)SemaphoreReducePermitsCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(reduction);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SemaphoreReducePermitsCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        int32_t reduction) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(reduction);
                    return dataSize;
                }

                SemaphoreReducePermitsCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("SemaphoreReducePermitsCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }
                }

                SemaphoreReducePermitsCodec::ResponseParameters SemaphoreReducePermitsCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return SemaphoreReducePermitsCodec::ResponseParameters(clientMessage);
                }

                SemaphoreReducePermitsCodec::ResponseParameters::ResponseParameters(const SemaphoreReducePermitsCodec::ResponseParameters &rhs) {
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

