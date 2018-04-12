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



#include "hazelcast/client/protocol/codec/MapRemoveAllCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapRemoveAllCodec::RequestParameters::TYPE = HZ_MAP_REMOVEALL;
                const bool MapRemoveAllCodec::RequestParameters::RETRYABLE = false;
                const int32_t MapRemoveAllCodec::ResponseParameters::TYPE = 100;
                std::auto_ptr<ClientMessage> MapRemoveAllCodec::RequestParameters::encode(
                        const std::string &name, 
                        const serialization::pimpl::Data &predicate) {
                    int32_t requiredDataSize = calculateDataSize(name, predicate);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)MapRemoveAllCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(predicate);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapRemoveAllCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const serialization::pimpl::Data &predicate) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    return dataSize;
                }

                MapRemoveAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("MapRemoveAllCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }
                }

                MapRemoveAllCodec::ResponseParameters MapRemoveAllCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapRemoveAllCodec::ResponseParameters(clientMessage);
                }

                MapRemoveAllCodec::ResponseParameters::ResponseParameters(const MapRemoveAllCodec::ResponseParameters &rhs) {
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

