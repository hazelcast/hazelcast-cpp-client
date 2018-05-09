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

#include "hazelcast/client/protocol/codec/AtomicLongGetCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const AtomicLongMessageType AtomicLongGetCodec::REQUEST_TYPE = HZ_ATOMICLONG_GET;
                const bool AtomicLongGetCodec::RETRYABLE = false;
                const ResponseMessageConst AtomicLongGetCodec::RESPONSE_TYPE = (ResponseMessageConst) 103;

                std::auto_ptr<ClientMessage> AtomicLongGetCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) AtomicLongGetCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t AtomicLongGetCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                AtomicLongGetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int64_t>();

                }

                AtomicLongGetCodec::ResponseParameters
                AtomicLongGetCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return AtomicLongGetCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

