/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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



#include "hazelcast/client/protocol/codec/MapKeySetWithPagingPredicateCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapKeySetWithPagingPredicateCodec::RequestParameters::TYPE = HZ_MAP_KEYSETWITHPAGINGPREDICATE;
                const bool MapKeySetWithPagingPredicateCodec::RequestParameters::RETRYABLE = false;
                const int32_t MapKeySetWithPagingPredicateCodec::ResponseParameters::TYPE = 106;
                std::auto_ptr<ClientMessage> MapKeySetWithPagingPredicateCodec::RequestParameters::encode(
                        const std::string &name, 
                        const serialization::pimpl::Data &predicate) {
                    int32_t requiredDataSize = calculateDataSize(name, predicate);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)MapKeySetWithPagingPredicateCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(predicate);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapKeySetWithPagingPredicateCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const serialization::pimpl::Data &predicate) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    return dataSize;
                }

                MapKeySetWithPagingPredicateCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("MapKeySetWithPagingPredicateCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.getArray<serialization::pimpl::Data >();
                }

                MapKeySetWithPagingPredicateCodec::ResponseParameters MapKeySetWithPagingPredicateCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapKeySetWithPagingPredicateCodec::ResponseParameters(clientMessage);
                }

                MapKeySetWithPagingPredicateCodec::ResponseParameters::ResponseParameters(const MapKeySetWithPagingPredicateCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

