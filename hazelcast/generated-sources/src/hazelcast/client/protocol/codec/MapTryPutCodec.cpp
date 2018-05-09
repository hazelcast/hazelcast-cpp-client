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

#include "hazelcast/client/protocol/codec/MapTryPutCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapTryPutCodec::REQUEST_TYPE = HZ_MAP_TRYPUT;
                const bool MapTryPutCodec::RETRYABLE = false;
                const ResponseMessageConst MapTryPutCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::auto_ptr<ClientMessage> MapTryPutCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t timeout) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, threadId, timeout);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapTryPutCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->set(timeout);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapTryPutCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t timeout) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(timeout);
                    return dataSize;
                }

                MapTryPutCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MapTryPutCodec::ResponseParameters
                MapTryPutCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapTryPutCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

