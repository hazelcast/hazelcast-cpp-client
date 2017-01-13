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



#include "hazelcast/client/protocol/codec/ClientDestroyProxyCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientDestroyProxyCodec::RequestParameters::TYPE = HZ_CLIENT_DESTROYPROXY;
                const bool ClientDestroyProxyCodec::RequestParameters::RETRYABLE = false;
                const int32_t ClientDestroyProxyCodec::ResponseParameters::TYPE = 100;
                std::auto_ptr<ClientMessage> ClientDestroyProxyCodec::RequestParameters::encode(
                        const std::string &name, 
                        const std::string &serviceName) {
                    int32_t requiredDataSize = calculateDataSize(name, serviceName);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ClientDestroyProxyCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(serviceName);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientDestroyProxyCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const std::string &serviceName) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(serviceName);
                    return dataSize;
                }

                ClientDestroyProxyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ClientDestroyProxyCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }
                }

                ClientDestroyProxyCodec::ResponseParameters ClientDestroyProxyCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientDestroyProxyCodec::ResponseParameters(clientMessage);
                }

                ClientDestroyProxyCodec::ResponseParameters::ResponseParameters(const ClientDestroyProxyCodec::ResponseParameters &rhs) {
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

