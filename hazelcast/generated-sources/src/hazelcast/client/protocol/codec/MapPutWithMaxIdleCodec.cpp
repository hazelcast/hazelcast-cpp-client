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

#include "hazelcast/client/protocol/codec/MapPutWithMaxIdleCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapPutWithMaxIdleCodec::REQUEST_TYPE = HZ_MAP_PUTWITHMAXIDLE;
                const bool MapPutWithMaxIdleCodec::RETRYABLE = false;
                const ResponseMessageConst MapPutWithMaxIdleCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::auto_ptr<ClientMessage> MapPutWithMaxIdleCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t ttl,
                        int64_t maxIdle) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, threadId, ttl, maxIdle);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapPutWithMaxIdleCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->set(ttl);
                    clientMessage->set(maxIdle);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapPutWithMaxIdleCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t ttl,
                        int64_t maxIdle) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(ttl);
                    dataSize += ClientMessage::calculateDataSize(maxIdle);
                    return dataSize;
                }

                MapPutWithMaxIdleCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                MapPutWithMaxIdleCodec::ResponseParameters
                MapPutWithMaxIdleCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapPutWithMaxIdleCodec::ResponseParameters(clientMessage);
                }

                MapPutWithMaxIdleCodec::ResponseParameters::ResponseParameters(
                        const MapPutWithMaxIdleCodec::ResponseParameters &rhs) {
                    response = std::auto_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

