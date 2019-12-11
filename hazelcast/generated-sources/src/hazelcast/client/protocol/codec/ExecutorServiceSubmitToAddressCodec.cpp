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

#include "hazelcast/client/protocol/codec/ExecutorServiceSubmitToAddressCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/Address.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ExecutorServiceMessageType ExecutorServiceSubmitToAddressCodec::REQUEST_TYPE = HZ_EXECUTORSERVICE_SUBMITTOADDRESS;
                const bool ExecutorServiceSubmitToAddressCodec::RETRYABLE = false;
                const ResponseMessageConst ExecutorServiceSubmitToAddressCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::auto_ptr<ClientMessage> ExecutorServiceSubmitToAddressCodec::encodeRequest(
                        const std::string &name, 
                        const std::string &uuid, 
                        const serialization::pimpl::Data &callable, 
                        const Address &address) {
                    int32_t requiredDataSize = calculateDataSize(name, uuid, callable, address);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ExecutorServiceSubmitToAddressCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(uuid);
                    clientMessage->set(callable);
                    clientMessage->set(address);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ExecutorServiceSubmitToAddressCodec::calculateDataSize(
                        const std::string &name, 
                        const std::string &uuid, 
                        const serialization::pimpl::Data &callable, 
                        const Address &address) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(uuid);
                    dataSize += ClientMessage::calculateDataSize(callable);
                    dataSize += ClientMessage::calculateDataSize(address);
                    return dataSize;
                }

                    ExecutorServiceSubmitToAddressCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                        
                        

                    response = clientMessage.getNullable<serialization::pimpl::Data >();
                            
                    }

                    ExecutorServiceSubmitToAddressCodec::ResponseParameters ExecutorServiceSubmitToAddressCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                        return ExecutorServiceSubmitToAddressCodec::ResponseParameters(clientMessage);
                    }

                    ExecutorServiceSubmitToAddressCodec::ResponseParameters::ResponseParameters(const ExecutorServiceSubmitToAddressCodec::ResponseParameters &rhs) {
                            response = std::auto_ptr<serialization::pimpl::Data >(new serialization::pimpl::Data(*rhs.response));
                    }

            }
        }
    }
}

