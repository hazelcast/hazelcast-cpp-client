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



#include "hazelcast/client/protocol/codec/MapRemoveInterceptorCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapRemoveInterceptorCodec::RequestParameters::TYPE = HZ_MAP_REMOVEINTERCEPTOR;
                const bool MapRemoveInterceptorCodec::RequestParameters::RETRYABLE = false;
                const int32_t MapRemoveInterceptorCodec::ResponseParameters::TYPE = 101;
                std::auto_ptr<ClientMessage> MapRemoveInterceptorCodec::RequestParameters::encode(
                        const std::string &name, 
                        const std::string &id) {
                    int32_t requiredDataSize = calculateDataSize(name, id);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)MapRemoveInterceptorCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(id);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapRemoveInterceptorCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const std::string &id) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(id);
                    return dataSize;
                }

                MapRemoveInterceptorCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("MapRemoveInterceptorCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<bool >();
                }

                MapRemoveInterceptorCodec::ResponseParameters MapRemoveInterceptorCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapRemoveInterceptorCodec::ResponseParameters(clientMessage);
                }

                MapRemoveInterceptorCodec::ResponseParameters::ResponseParameters(const MapRemoveInterceptorCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

