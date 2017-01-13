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



#include "hazelcast/client/protocol/codec/MultiMapClearCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapClearCodec::RequestParameters::TYPE = HZ_MULTIMAP_CLEAR;
                const bool MultiMapClearCodec::RequestParameters::RETRYABLE = false;
                const int32_t MultiMapClearCodec::ResponseParameters::TYPE = 100;
                std::auto_ptr<ClientMessage> MultiMapClearCodec::RequestParameters::encode(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)MultiMapClearCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapClearCodec::RequestParameters::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                MultiMapClearCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("MultiMapClearCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }
                }

                MultiMapClearCodec::ResponseParameters MultiMapClearCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MultiMapClearCodec::ResponseParameters(clientMessage);
                }

                MultiMapClearCodec::ResponseParameters::ResponseParameters(const MultiMapClearCodec::ResponseParameters &rhs) {
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

