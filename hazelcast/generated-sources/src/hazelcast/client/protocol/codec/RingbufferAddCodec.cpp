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



#include "hazelcast/client/protocol/codec/RingbufferAddCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const RingbufferMessageType RingbufferAddCodec::RequestParameters::TYPE = HZ_RINGBUFFER_ADD;
                const bool RingbufferAddCodec::RequestParameters::RETRYABLE = false;
                const int32_t RingbufferAddCodec::ResponseParameters::TYPE = 103;
                std::auto_ptr<ClientMessage> RingbufferAddCodec::RequestParameters::encode(
                        const std::string &name, 
                        int32_t overflowPolicy, 
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, overflowPolicy, value);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)RingbufferAddCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(overflowPolicy);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t RingbufferAddCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        int32_t overflowPolicy, 
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(overflowPolicy);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                RingbufferAddCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("RingbufferAddCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<int64_t >();
                }

                RingbufferAddCodec::ResponseParameters RingbufferAddCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return RingbufferAddCodec::ResponseParameters(clientMessage);
                }

                RingbufferAddCodec::ResponseParameters::ResponseParameters(const RingbufferAddCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

