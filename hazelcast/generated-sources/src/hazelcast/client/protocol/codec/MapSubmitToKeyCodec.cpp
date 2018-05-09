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

#include "hazelcast/client/protocol/codec/MapSubmitToKeyCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapSubmitToKeyCodec::REQUEST_TYPE = HZ_MAP_SUBMITTOKEY;
                const bool MapSubmitToKeyCodec::RETRYABLE = false;
                const ResponseMessageConst MapSubmitToKeyCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::auto_ptr<ClientMessage> MapSubmitToKeyCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &entryProcessor,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, entryProcessor, key, threadId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapSubmitToKeyCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(entryProcessor);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapSubmitToKeyCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &entryProcessor,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(entryProcessor);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MapSubmitToKeyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                MapSubmitToKeyCodec::ResponseParameters
                MapSubmitToKeyCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapSubmitToKeyCodec::ResponseParameters(clientMessage);
                }

                MapSubmitToKeyCodec::ResponseParameters::ResponseParameters(
                        const MapSubmitToKeyCodec::ResponseParameters &rhs) {
                    response = std::auto_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(*rhs.response));
                }
            }
        }
    }
}

