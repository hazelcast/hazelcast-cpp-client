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

#include "hazelcast/client/protocol/codec/ExecutorServiceCancelOnPartitionCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ExecutorServiceMessageType ExecutorServiceCancelOnPartitionCodec::REQUEST_TYPE = HZ_EXECUTORSERVICE_CANCELONPARTITION;
                const bool ExecutorServiceCancelOnPartitionCodec::RETRYABLE = false;
                const ResponseMessageConst ExecutorServiceCancelOnPartitionCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::auto_ptr<ClientMessage> ExecutorServiceCancelOnPartitionCodec::encodeRequest(
                        const std::string &uuid, 
                        int32_t partitionId, 
                        bool interrupt) {
                    int32_t requiredDataSize = calculateDataSize(uuid, partitionId, interrupt);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ExecutorServiceCancelOnPartitionCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(uuid);
                    clientMessage->set(partitionId);
                    clientMessage->set(interrupt);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ExecutorServiceCancelOnPartitionCodec::calculateDataSize(
                        const std::string &uuid, 
                        int32_t partitionId, 
                        bool interrupt) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(uuid);
                    dataSize += ClientMessage::calculateDataSize(partitionId);
                    dataSize += ClientMessage::calculateDataSize(interrupt);
                    return dataSize;
                }

                    ExecutorServiceCancelOnPartitionCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                        
                        

                    response = clientMessage.get<bool >();
                            
                    }

                    ExecutorServiceCancelOnPartitionCodec::ResponseParameters ExecutorServiceCancelOnPartitionCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                        return ExecutorServiceCancelOnPartitionCodec::ResponseParameters(clientMessage);
                    }


            }
        }
    }
}

