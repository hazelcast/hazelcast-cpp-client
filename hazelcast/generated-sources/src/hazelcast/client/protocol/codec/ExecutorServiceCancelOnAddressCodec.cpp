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

#include "hazelcast/client/protocol/codec/ExecutorServiceCancelOnAddressCodec.h"
#include "hazelcast/client/Address.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ExecutorServiceMessageType ExecutorServiceCancelOnAddressCodec::REQUEST_TYPE = HZ_EXECUTORSERVICE_CANCELONADDRESS;
                const bool ExecutorServiceCancelOnAddressCodec::RETRYABLE = false;
                const ResponseMessageConst ExecutorServiceCancelOnAddressCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::auto_ptr<ClientMessage> ExecutorServiceCancelOnAddressCodec::encodeRequest(
                        const std::string &uuid, 
                        const Address &address, 
                        bool interrupt) {
                    int32_t requiredDataSize = calculateDataSize(uuid, address, interrupt);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ExecutorServiceCancelOnAddressCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(uuid);
                    clientMessage->set(address);
                    clientMessage->set(interrupt);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ExecutorServiceCancelOnAddressCodec::calculateDataSize(
                        const std::string &uuid, 
                        const Address &address, 
                        bool interrupt) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(uuid);
                    dataSize += ClientMessage::calculateDataSize(address);
                    dataSize += ClientMessage::calculateDataSize(interrupt);
                    return dataSize;
                }

                    ExecutorServiceCancelOnAddressCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                        
                        

                    response = clientMessage.get<bool >();
                            
                    }

                    ExecutorServiceCancelOnAddressCodec::ResponseParameters ExecutorServiceCancelOnAddressCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                        return ExecutorServiceCancelOnAddressCodec::ResponseParameters(clientMessage);
                    }


            }
        }
    }
}

