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



#include "hazelcast/client/protocol/codec/ClientRemoveAllListenersCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientRemoveAllListenersCodec::RequestParameters::TYPE = HZ_CLIENT_REMOVEALLLISTENERS;
                const bool ClientRemoveAllListenersCodec::RequestParameters::RETRYABLE = false;
                const int32_t ClientRemoveAllListenersCodec::ResponseParameters::TYPE = 100;
                std::auto_ptr<ClientMessage> ClientRemoveAllListenersCodec::RequestParameters::encode() {
                    int32_t requiredDataSize = calculateDataSize();
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ClientRemoveAllListenersCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientRemoveAllListenersCodec::RequestParameters::calculateDataSize() {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    return dataSize;
                }

                ClientRemoveAllListenersCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ClientRemoveAllListenersCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }
                }

                ClientRemoveAllListenersCodec::ResponseParameters ClientRemoveAllListenersCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientRemoveAllListenersCodec::ResponseParameters(clientMessage);
                }

                ClientRemoveAllListenersCodec::ResponseParameters::ResponseParameters(const ClientRemoveAllListenersCodec::ResponseParameters &rhs) {
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

