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

#include "hazelcast/client/protocol/codec/RingbufferReadManyCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const RingbufferMessageType RingbufferReadManyCodec::REQUEST_TYPE = HZ_RINGBUFFER_READMANY;
                const bool RingbufferReadManyCodec::RETRYABLE = true;
                const ResponseMessageConst RingbufferReadManyCodec::RESPONSE_TYPE = (ResponseMessageConst) 115;

                std::auto_ptr<ClientMessage> RingbufferReadManyCodec::encodeRequest(
                        const std::string &name,
                        int64_t startSequence,
                        int32_t minCount,
                        int32_t maxCount,
                        const serialization::pimpl::Data *filter) {
                    int32_t requiredDataSize = calculateDataSize(name, startSequence, minCount, maxCount, filter);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) RingbufferReadManyCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(startSequence);
                    clientMessage->set(minCount);
                    clientMessage->set(maxCount);
                    clientMessage->set(filter);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t RingbufferReadManyCodec::calculateDataSize(
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
                    if (RESPONSE_TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException(
                                "RingbufferReadManyCodec::ResponseParameters::decode", clientMessage.getMessageType(),
                                RESPONSE_TYPE);
                    }

                    itemSeqsExist = false;
                    nextSeqExist = false;


                    readCount = clientMessage.get<int32_t>();


                    items = clientMessage.getArray<serialization::pimpl::Data>();
                    if (clientMessage.isComplete()) {
                        return;
                    }

                    itemSeqs = clientMessage.getNullableArray<int64_t>();
                    itemSeqsExist = true;
                    if (clientMessage.isComplete()) {
                        return;
                    }

                    nextSeq = clientMessage.get<int64_t>();
                    nextSeqExist = true;
                }

                RingbufferReadManyCodec::ResponseParameters
                RingbufferReadManyCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return RingbufferReadManyCodec::ResponseParameters(clientMessage);
                }

                RingbufferReadManyCodec::ResponseParameters::ResponseParameters(
                        const RingbufferReadManyCodec::ResponseParameters &rhs) {
                    readCount = rhs.readCount;
                    items = rhs.items;
                    itemSeqs = std::auto_ptr<std::vector<int64_t> >(new std::vector<int64_t>(*rhs.itemSeqs));
                    nextSeq = rhs.nextSeq;
                }
            }
        }
    }
}

