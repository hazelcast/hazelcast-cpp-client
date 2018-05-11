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

#include "hazelcast/client/protocol/codec/AtomicLongSetCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const AtomicLongMessageType AtomicLongSetCodec::REQUEST_TYPE = HZ_ATOMICLONG_SET;
                const bool AtomicLongSetCodec::RETRYABLE = false;
                const ResponseMessageConst AtomicLongSetCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::auto_ptr<ClientMessage> AtomicLongSetCodec::encodeRequest(
                        const std::string &name,
                        int64_t newValue) {
                    int32_t requiredDataSize = calculateDataSize(name, newValue);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) AtomicLongSetCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(newValue);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t AtomicLongSetCodec::calculateDataSize(
                        const std::string &name,
                        int64_t newValue) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(newValue);
                    return dataSize;
                }


            }
        }
    }
}

