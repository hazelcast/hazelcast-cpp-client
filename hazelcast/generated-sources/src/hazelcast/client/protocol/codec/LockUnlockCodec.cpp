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



#include "hazelcast/client/protocol/codec/LockUnlockCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const LockMessageType LockUnlockCodec::RequestParameters::TYPE = HZ_LOCK_UNLOCK;
                const bool LockUnlockCodec::RequestParameters::RETRYABLE = false;
                const int32_t LockUnlockCodec::ResponseParameters::TYPE = 100;
                std::auto_ptr<ClientMessage> LockUnlockCodec::RequestParameters::encode(
                        const std::string &name, 
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, threadId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)LockUnlockCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t LockUnlockCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                LockUnlockCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("LockUnlockCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }
                }

                LockUnlockCodec::ResponseParameters LockUnlockCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return LockUnlockCodec::ResponseParameters(clientMessage);
                }

                LockUnlockCodec::ResponseParameters::ResponseParameters(const LockUnlockCodec::ResponseParameters &rhs) {
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

