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

#include "hazelcast/client/protocol/codec/MapAddIndexCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapAddIndexCodec::REQUEST_TYPE = HZ_MAP_ADDINDEX;
                const bool MapAddIndexCodec::RETRYABLE = false;
                const ResponseMessageConst MapAddIndexCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MapAddIndexCodec::encodeRequest(
                        const std::string &name,
                        const std::string &attribute,
                        bool ordered) {
                    int32_t requiredDataSize = calculateDataSize(name, attribute, ordered);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapAddIndexCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(attribute);
                    clientMessage->set(ordered);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapAddIndexCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &attribute,
                        bool ordered) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(attribute);
                    dataSize += ClientMessage::calculateDataSize(ordered);
                    return dataSize;
                }


            }
        }
    }
}

