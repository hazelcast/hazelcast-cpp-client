/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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



#include "hazelcast/client/protocol/codec/MapLoadAllCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapLoadAllCodec::RequestParameters::TYPE = HZ_MAP_LOADALL;
                const bool MapLoadAllCodec::RequestParameters::RETRYABLE = false;
                const int32_t MapLoadAllCodec::ResponseParameters::TYPE = 100;
                std::auto_ptr<ClientMessage> MapLoadAllCodec::RequestParameters::encode(
                        const std::string &name, 
                        bool replaceExistingValues) {
                    int32_t requiredDataSize = calculateDataSize(name, replaceExistingValues);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)MapLoadAllCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(replaceExistingValues);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapLoadAllCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        bool replaceExistingValues) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(replaceExistingValues);
                    return dataSize;
                }

                MapLoadAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("MapLoadAllCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }
                }

                MapLoadAllCodec::ResponseParameters MapLoadAllCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapLoadAllCodec::ResponseParameters(clientMessage);
                }

                MapLoadAllCodec::ResponseParameters::ResponseParameters(const MapLoadAllCodec::ResponseParameters &rhs) {
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

