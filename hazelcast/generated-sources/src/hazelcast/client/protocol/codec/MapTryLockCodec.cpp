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

#include "hazelcast/client/protocol/codec/MapTryLockCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapTryLockCodec::REQUEST_TYPE = HZ_MAP_TRYLOCK;
                const bool MapTryLockCodec::RETRYABLE = true;
                const ResponseMessageConst MapTryLockCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MapTryLockCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t lease,
                        int64_t timeout,
                        int64_t referenceId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId, lease, timeout, referenceId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapTryLockCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->set(lease);
                    clientMessage->set(timeout);
                    clientMessage->set(referenceId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapTryLockCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t lease,
                        int64_t timeout,
                        int64_t referenceId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(lease);
                    dataSize += ClientMessage::calculateDataSize(timeout);
                    dataSize += ClientMessage::calculateDataSize(referenceId);
                    return dataSize;
                }

                MapTryLockCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MapTryLockCodec::ResponseParameters
                MapTryLockCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapTryLockCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

