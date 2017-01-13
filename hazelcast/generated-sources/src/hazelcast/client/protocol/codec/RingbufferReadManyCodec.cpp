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



#include "hazelcast/client/protocol/codec/RingbufferReadManyCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const RingbufferMessageType RingbufferReadManyCodec::RequestParameters::TYPE = HZ_RINGBUFFER_READMANY;
                const bool RingbufferReadManyCodec::RequestParameters::RETRYABLE = false;
                const int32_t RingbufferReadManyCodec::ResponseParameters::TYPE = 115;
                std::auto_ptr<ClientMessage> RingbufferReadManyCodec::RequestParameters::encode(
                        const std::string &name, 
                        int64_t startSequence, 
                        int32_t minCount, 
                        int32_t maxCount, 
                        const serialization::pimpl::Data *filter) {
                    int32_t requiredDataSize = calculateDataSize(name, startSequence, minCount, maxCount, filter);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)RingbufferReadManyCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(startSequence);
                    clientMessage->set(minCount);
                    clientMessage->set(maxCount);
                    clientMessage->set(filter);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t RingbufferReadManyCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        int64_t startSequence, 
                        int32_t minCount, 
                        int32_t maxCount, 
                        const serialization::pimpl::Data *filter) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(startSequence);
                    dataSize += ClientMessage::calculateDataSize(minCount);
                    dataSize += ClientMessage::calculateDataSize(maxCount);
                    dataSize += ClientMessage::calculateDataSize(filter);
                    return dataSize;
                }

                RingbufferReadManyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("RingbufferReadManyCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    readCount = clientMessage.get<int32_t >();

                    items = clientMessage.getArray<serialization::pimpl::Data >();
                }

                RingbufferReadManyCodec::ResponseParameters RingbufferReadManyCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return RingbufferReadManyCodec::ResponseParameters(clientMessage);
                }

                RingbufferReadManyCodec::ResponseParameters::ResponseParameters(const RingbufferReadManyCodec::ResponseParameters &rhs) {
                        readCount = rhs.readCount;
                        items = rhs.items;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

