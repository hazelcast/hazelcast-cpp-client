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



#include "hazelcast/client/protocol/codec/ClientPingCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientPingCodec::RequestParameters::TYPE = HZ_CLIENT_PING;
                const bool ClientPingCodec::RequestParameters::RETRYABLE = true;
                const int32_t ClientPingCodec::ResponseParameters::TYPE = 100;
                std::auto_ptr<ClientMessage> ClientPingCodec::RequestParameters::encode() {
                    int32_t requiredDataSize = calculateDataSize();
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ClientPingCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientPingCodec::RequestParameters::calculateDataSize() {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    return dataSize;
                }

                ClientPingCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ClientPingCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }
                }

                ClientPingCodec::ResponseParameters ClientPingCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientPingCodec::ResponseParameters(clientMessage);
                }

                ClientPingCodec::ResponseParameters::ResponseParameters(const ClientPingCodec::ResponseParameters &rhs) {
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

