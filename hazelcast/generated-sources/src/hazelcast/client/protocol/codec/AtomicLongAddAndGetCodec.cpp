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

#include "hazelcast/client/protocol/codec/AtomicLongAddAndGetCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const AtomicLongMessageType AtomicLongAddAndGetCodec::REQUEST_TYPE = HZ_ATOMICLONG_ADDANDGET;
                const bool AtomicLongAddAndGetCodec::RETRYABLE = false;
                const ResponseMessageConst AtomicLongAddAndGetCodec::RESPONSE_TYPE = (ResponseMessageConst) 103;

                std::auto_ptr<ClientMessage> AtomicLongAddAndGetCodec::encodeRequest(
                        const std::string &name,
                        int64_t delta) {
                    int32_t requiredDataSize = calculateDataSize(name, delta);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) AtomicLongAddAndGetCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(delta);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t AtomicLongAddAndGetCodec::calculateDataSize(
                        const std::string &name,
                        int64_t delta) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(delta);
                    return dataSize;
                }

                AtomicLongAddAndGetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int64_t>();

                }

                AtomicLongAddAndGetCodec::ResponseParameters
                AtomicLongAddAndGetCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return AtomicLongAddAndGetCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

