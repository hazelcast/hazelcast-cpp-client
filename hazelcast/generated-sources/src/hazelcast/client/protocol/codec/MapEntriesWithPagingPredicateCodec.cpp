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



#include "hazelcast/client/protocol/codec/MapEntriesWithPagingPredicateCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapEntriesWithPagingPredicateCodec::RequestParameters::TYPE = HZ_MAP_ENTRIESWITHPAGINGPREDICATE;
                const bool MapEntriesWithPagingPredicateCodec::RequestParameters::RETRYABLE = false;
                const int32_t MapEntriesWithPagingPredicateCodec::ResponseParameters::TYPE = 117;
                std::auto_ptr<ClientMessage> MapEntriesWithPagingPredicateCodec::RequestParameters::encode(
                        const std::string &name, 
                        const serialization::pimpl::Data &predicate) {
                    int32_t requiredDataSize = calculateDataSize(name, predicate);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)MapEntriesWithPagingPredicateCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(predicate);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapEntriesWithPagingPredicateCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const serialization::pimpl::Data &predicate) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    return dataSize;
                }

                MapEntriesWithPagingPredicateCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("MapEntriesWithPagingPredicateCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.getArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > >();
                }

                MapEntriesWithPagingPredicateCodec::ResponseParameters MapEntriesWithPagingPredicateCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapEntriesWithPagingPredicateCodec::ResponseParameters(clientMessage);
                }

                MapEntriesWithPagingPredicateCodec::ResponseParameters::ResponseParameters(const MapEntriesWithPagingPredicateCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

