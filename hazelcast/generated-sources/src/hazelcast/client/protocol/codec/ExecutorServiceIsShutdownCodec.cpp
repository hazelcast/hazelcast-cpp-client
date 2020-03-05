/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/protocol/codec/ExecutorServiceIsShutdownCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ExecutorServiceMessageType ExecutorServiceIsShutdownCodec::REQUEST_TYPE = HZ_EXECUTORSERVICE_ISSHUTDOWN;
                const bool ExecutorServiceIsShutdownCodec::RETRYABLE = false;
                const ResponseMessageConst ExecutorServiceIsShutdownCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ExecutorServiceIsShutdownCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ExecutorServiceIsShutdownCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ExecutorServiceIsShutdownCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                    ExecutorServiceIsShutdownCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                        
                        

                    response = clientMessage.get<bool >();
                            
                    }

                    ExecutorServiceIsShutdownCodec::ResponseParameters ExecutorServiceIsShutdownCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                        return ExecutorServiceIsShutdownCodec::ResponseParameters(clientMessage);
                    }


            }
        }
    }
}

