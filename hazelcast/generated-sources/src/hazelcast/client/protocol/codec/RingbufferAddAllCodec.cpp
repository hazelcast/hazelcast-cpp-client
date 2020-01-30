/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/protocol/codec/RingbufferAddAllCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const RingbufferMessageType RingbufferAddAllCodec::REQUEST_TYPE = HZ_RINGBUFFER_ADDALL;
                const bool RingbufferAddAllCodec::RETRYABLE = false;
                const ResponseMessageConst RingbufferAddAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 103;

                std::unique_ptr<ClientMessage> RingbufferAddAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &valueList,
                        int32_t overflowPolicy) {
                    int32_t requiredDataSize = calculateDataSize(name, valueList, overflowPolicy);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) RingbufferAddAllCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(valueList);
                    clientMessage->set(overflowPolicy);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t RingbufferAddAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &valueList,
                        int32_t overflowPolicy) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(valueList);
                    dataSize += ClientMessage::calculateDataSize(overflowPolicy);
                    return dataSize;
                }

                RingbufferAddAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int64_t>();

                }

                RingbufferAddAllCodec::ResponseParameters
                RingbufferAddAllCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return RingbufferAddAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

