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

#include "hazelcast/client/protocol/codec/LockLockCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const LockMessageType LockLockCodec::REQUEST_TYPE = HZ_LOCK_LOCK;
                const bool LockLockCodec::RETRYABLE = true;
                const ResponseMessageConst LockLockCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::auto_ptr<ClientMessage> LockLockCodec::encodeRequest(
                        const std::string &name,
                        int64_t leaseTime,
                        int64_t threadId,
                        int64_t referenceId) {
                    int32_t requiredDataSize = calculateDataSize(name, leaseTime, threadId, referenceId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) LockLockCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(leaseTime);
                    clientMessage->set(threadId);
                    clientMessage->set(referenceId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t LockLockCodec::calculateDataSize(
                        const std::string &name,
                        int64_t leaseTime,
                        int64_t threadId,
                        int64_t referenceId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(leaseTime);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(referenceId);
                    return dataSize;
                }

                LockLockCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                }

                LockLockCodec::ResponseParameters
                LockLockCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return LockLockCodec::ResponseParameters(clientMessage);
                }

                LockLockCodec::ResponseParameters::ResponseParameters(const LockLockCodec::ResponseParameters &rhs) {
                }
            }
        }
    }
}

