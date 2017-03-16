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



#include "hazelcast/client/protocol/codec/MapExecuteOnKeysCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapExecuteOnKeysCodec::RequestParameters::TYPE = HZ_MAP_EXECUTEONKEYS;
                const bool MapExecuteOnKeysCodec::RequestParameters::RETRYABLE = false;
                const int32_t MapExecuteOnKeysCodec::ResponseParameters::TYPE = 117;
                std::auto_ptr<ClientMessage> MapExecuteOnKeysCodec::RequestParameters::encode(
                        const std::string &name, 
                        const serialization::pimpl::Data &entryProcessor, 
                        const std::vector<serialization::pimpl::Data > &keys) {
                    int32_t requiredDataSize = calculateDataSize(name, entryProcessor, keys);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)MapExecuteOnKeysCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(entryProcessor);
                    clientMessage->setArray<serialization::pimpl::Data >(keys);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapExecuteOnKeysCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const serialization::pimpl::Data &entryProcessor, 
                        const std::vector<serialization::pimpl::Data > &keys) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(entryProcessor);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data >(keys);
                    return dataSize;
                }

                MapExecuteOnKeysCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("MapExecuteOnKeysCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.getArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > >();
                }

                MapExecuteOnKeysCodec::ResponseParameters MapExecuteOnKeysCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapExecuteOnKeysCodec::ResponseParameters(clientMessage);
                }

                MapExecuteOnKeysCodec::ResponseParameters::ResponseParameters(const MapExecuteOnKeysCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

