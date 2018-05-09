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

#include "hazelcast/client/protocol/codec/MapPutAllCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapPutAllCodec::REQUEST_TYPE = HZ_MAP_PUTALL;
                const bool MapPutAllCodec::RETRYABLE = false;
                const ResponseMessageConst MapPutAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::auto_ptr<ClientMessage> MapPutAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &entries) {
                    int32_t requiredDataSize = calculateDataSize(name, entries);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapPutAllCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >(
                            entries);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapPutAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &entries) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >(
                            entries);
                    return dataSize;
                }

                MapPutAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                }

                MapPutAllCodec::ResponseParameters
                MapPutAllCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapPutAllCodec::ResponseParameters(clientMessage);
                }

                MapPutAllCodec::ResponseParameters::ResponseParameters(const MapPutAllCodec::ResponseParameters &rhs) {
                }
            }
        }
    }
}

