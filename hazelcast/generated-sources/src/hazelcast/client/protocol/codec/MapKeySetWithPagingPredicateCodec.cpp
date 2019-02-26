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

#include "hazelcast/client/protocol/codec/MapKeySetWithPagingPredicateCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapKeySetWithPagingPredicateCodec::REQUEST_TYPE = HZ_MAP_KEYSETWITHPAGINGPREDICATE;
                const bool MapKeySetWithPagingPredicateCodec::RETRYABLE = true;
                const ResponseMessageConst MapKeySetWithPagingPredicateCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::auto_ptr<ClientMessage> MapKeySetWithPagingPredicateCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t requiredDataSize = calculateDataSize(name, predicate);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapKeySetWithPagingPredicateCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(predicate);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapKeySetWithPagingPredicateCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    return dataSize;
                }

                MapKeySetWithPagingPredicateCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                MapKeySetWithPagingPredicateCodec::ResponseParameters
                MapKeySetWithPagingPredicateCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapKeySetWithPagingPredicateCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

