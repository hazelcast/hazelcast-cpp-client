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



#include "hazelcast/client/protocol/codec/LockLockCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const LockMessageType LockLockCodec::RequestParameters::TYPE = HZ_LOCK_LOCK;
                const bool LockLockCodec::RequestParameters::RETRYABLE = false;
                const int32_t LockLockCodec::ResponseParameters::TYPE = 100;
                std::auto_ptr<ClientMessage> LockLockCodec::RequestParameters::encode(
                        const std::string &name, 
                        int64_t leaseTime, 
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, leaseTime, threadId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)LockLockCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(leaseTime);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t LockLockCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        int64_t leaseTime, 
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(leaseTime);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                LockLockCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("LockLockCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }
                }

                LockLockCodec::ResponseParameters LockLockCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return LockLockCodec::ResponseParameters(clientMessage);
                }

                LockLockCodec::ResponseParameters::ResponseParameters(const LockLockCodec::ResponseParameters &rhs) {
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

