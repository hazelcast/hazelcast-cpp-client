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

#include "hazelcast/client/protocol/codec/MapReplaceIfSameCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapReplaceIfSameCodec::REQUEST_TYPE = HZ_MAP_REPLACEIFSAME;
                const bool MapReplaceIfSameCodec::RETRYABLE = false;
                const ResponseMessageConst MapReplaceIfSameCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::auto_ptr<ClientMessage> MapReplaceIfSameCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &testValue,
                        const serialization::pimpl::Data &value,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, testValue, value, threadId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapReplaceIfSameCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(testValue);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapReplaceIfSameCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &testValue,
                        const serialization::pimpl::Data &value,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(testValue);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MapReplaceIfSameCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MapReplaceIfSameCodec::ResponseParameters
                MapReplaceIfSameCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapReplaceIfSameCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

