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

#include "hazelcast/client/protocol/codec/LockTryLockCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const LockMessageType LockTryLockCodec::REQUEST_TYPE = HZ_LOCK_TRYLOCK;
                const bool LockTryLockCodec::RETRYABLE = true;
                const ResponseMessageConst LockTryLockCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::auto_ptr<ClientMessage> LockTryLockCodec::encodeRequest(
                        const std::string &name,
                        int64_t threadId,
                        int64_t lease,
                        int64_t timeout,
                        int64_t referenceId) {
                    int32_t requiredDataSize = calculateDataSize(name, threadId, lease, timeout, referenceId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) LockTryLockCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(threadId);
                    clientMessage->set(lease);
                    clientMessage->set(timeout);
                    clientMessage->set(referenceId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t LockTryLockCodec::calculateDataSize(
                        const std::string &name,
                        int64_t threadId,
                        int64_t lease,
                        int64_t timeout,
                        int64_t referenceId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(lease);
                    dataSize += ClientMessage::calculateDataSize(timeout);
                    dataSize += ClientMessage::calculateDataSize(referenceId);
                    return dataSize;
                }

                LockTryLockCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                LockTryLockCodec::ResponseParameters
                LockTryLockCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return LockTryLockCodec::ResponseParameters(clientMessage);
                }

                LockTryLockCodec::ResponseParameters::ResponseParameters(
                        const LockTryLockCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }
            }
        }
    }
}

