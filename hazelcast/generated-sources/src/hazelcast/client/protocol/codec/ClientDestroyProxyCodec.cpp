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

#include "hazelcast/util/Util.h"
#include "hazelcast/util/ILogger.h"

#include "hazelcast/client/protocol/codec/ClientDestroyProxyCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientDestroyProxyCodec::REQUEST_TYPE = HZ_CLIENT_DESTROYPROXY;
                const bool ClientDestroyProxyCodec::RETRYABLE = false;
                const ResponseMessageConst ClientDestroyProxyCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::auto_ptr<ClientMessage> ClientDestroyProxyCodec::encodeRequest(
                        const std::string &name,
                        const std::string &serviceName) {
                    int32_t requiredDataSize = calculateDataSize(name, serviceName);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientDestroyProxyCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(serviceName);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientDestroyProxyCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &serviceName) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(serviceName);
                    return dataSize;
                }

                ClientDestroyProxyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (RESPONSE_TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException(
                                "ClientDestroyProxyCodec::ResponseParameters::decode", clientMessage.getMessageType(),
                                RESPONSE_TYPE);
                    }


                }

                ClientDestroyProxyCodec::ResponseParameters
                ClientDestroyProxyCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientDestroyProxyCodec::ResponseParameters(clientMessage);
                }

                ClientDestroyProxyCodec::ResponseParameters::ResponseParameters(
                        const ClientDestroyProxyCodec::ResponseParameters &rhs) {
                }
            }
        }
    }
}

