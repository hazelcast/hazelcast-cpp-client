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

#include "hazelcast/client/protocol/codec/LockForceUnlockCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const LockMessageType LockForceUnlockCodec::REQUEST_TYPE = HZ_LOCK_FORCEUNLOCK;
                const bool LockForceUnlockCodec::RETRYABLE = true;
                const ResponseMessageConst LockForceUnlockCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::auto_ptr<ClientMessage> LockForceUnlockCodec::encodeRequest(
                        const std::string &name,
                        int64_t referenceId) {
                    int32_t requiredDataSize = calculateDataSize(name, referenceId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) LockForceUnlockCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(referenceId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t LockForceUnlockCodec::calculateDataSize(
                        const std::string &name,
                        int64_t referenceId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(referenceId);
                    return dataSize;
                }

                LockForceUnlockCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                }

                LockForceUnlockCodec::ResponseParameters
                LockForceUnlockCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return LockForceUnlockCodec::ResponseParameters(clientMessage);
                }

                LockForceUnlockCodec::ResponseParameters::ResponseParameters(
                        const LockForceUnlockCodec::ResponseParameters &rhs) {
                }
            }
        }
    }
}

