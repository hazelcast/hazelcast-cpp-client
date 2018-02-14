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



#include "hazelcast/client/protocol/codec/MapPutAllCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapPutAllCodec::RequestParameters::TYPE = HZ_MAP_PUTALL;
                const bool MapPutAllCodec::RequestParameters::RETRYABLE = false;
                const int32_t MapPutAllCodec::ResponseParameters::TYPE = 100;
                std::auto_ptr<ClientMessage> MapPutAllCodec::RequestParameters::encode(
                        const std::string &name, 
                        const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > > &entries) {
                    int32_t requiredDataSize = calculateDataSize(name, entries);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)MapPutAllCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setEntryArray<serialization::pimpl::Data, serialization::pimpl::Data >(entries);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapPutAllCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > > &entries) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data, serialization::pimpl::Data > (entries);
                    return dataSize;
                }

                MapPutAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("MapPutAllCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }
                }

                MapPutAllCodec::ResponseParameters MapPutAllCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapPutAllCodec::ResponseParameters(clientMessage);
                }

                MapPutAllCodec::ResponseParameters::ResponseParameters(const MapPutAllCodec::ResponseParameters &rhs) {
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

