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

#include "hazelcast/client/protocol/codec/MapGetAllCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapGetAllCodec::REQUEST_TYPE = HZ_MAP_GETALL;
                const bool MapGetAllCodec::RETRYABLE = false;
                const ResponseMessageConst MapGetAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 117;

                std::unique_ptr<ClientMessage> MapGetAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &keys) {
                    int32_t requiredDataSize = calculateDataSize(name, keys);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapGetAllCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(keys);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapGetAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &keys) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(keys);
                    return dataSize;
                }

                MapGetAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >();

                }

                MapGetAllCodec::ResponseParameters
                MapGetAllCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapGetAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

