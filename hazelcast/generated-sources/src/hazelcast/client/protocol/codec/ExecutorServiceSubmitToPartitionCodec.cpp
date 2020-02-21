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

#include "hazelcast/client/protocol/codec/ExecutorServiceSubmitToPartitionCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ExecutorServiceMessageType ExecutorServiceSubmitToPartitionCodec::REQUEST_TYPE = HZ_EXECUTORSERVICE_SUBMITTOPARTITION;
                const bool ExecutorServiceSubmitToPartitionCodec::RETRYABLE = false;
                const ResponseMessageConst ExecutorServiceSubmitToPartitionCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> ExecutorServiceSubmitToPartitionCodec::encodeRequest(
                        const std::string &name, 
                        const std::string &uuid, 
                        const serialization::pimpl::Data &callable, 
                        int32_t partitionId) {
                    int32_t requiredDataSize = calculateDataSize(name, uuid, callable, partitionId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ExecutorServiceSubmitToPartitionCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(uuid);
                    clientMessage->set(callable);
                    clientMessage->set(partitionId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ExecutorServiceSubmitToPartitionCodec::calculateDataSize(
                        const std::string &name, 
                        const std::string &uuid, 
                        const serialization::pimpl::Data &callable, 
                        int32_t partitionId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(uuid);
                    dataSize += ClientMessage::calculateDataSize(callable);
                    dataSize += ClientMessage::calculateDataSize(partitionId);
                    return dataSize;
                }

                    ExecutorServiceSubmitToPartitionCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                        
                        

                    response = clientMessage.getNullable<serialization::pimpl::Data >();
                            
                    }

                    ExecutorServiceSubmitToPartitionCodec::ResponseParameters ExecutorServiceSubmitToPartitionCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                        return ExecutorServiceSubmitToPartitionCodec::ResponseParameters(clientMessage);
                    }

                    ExecutorServiceSubmitToPartitionCodec::ResponseParameters::ResponseParameters(const ExecutorServiceSubmitToPartitionCodec::ResponseParameters &rhs) {
                            response = std::unique_ptr<serialization::pimpl::Data >(new serialization::pimpl::Data(*rhs.response));
                    }

            }
        }
    }
}

