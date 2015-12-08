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



#include "hazelcast/client/protocol/codec/ExecutorServiceSubmitToPartitionCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ExecutorServiceMessageType ExecutorServiceSubmitToPartitionCodec::RequestParameters::TYPE = HZ_EXECUTORSERVICE_SUBMITTOPARTITION;
                const bool ExecutorServiceSubmitToPartitionCodec::RequestParameters::RETRYABLE = false;
                const int32_t ExecutorServiceSubmitToPartitionCodec::ResponseParameters::TYPE = 105;
                std::auto_ptr<ClientMessage> ExecutorServiceSubmitToPartitionCodec::RequestParameters::encode(
                        const std::string &name, 
                        const std::string &uuid, 
                        const serialization::pimpl::Data &callable, 
                        int32_t partitionId) {
                    int32_t requiredDataSize = calculateDataSize(name, uuid, callable, partitionId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ExecutorServiceSubmitToPartitionCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(uuid);
                    clientMessage->set(callable);
                    clientMessage->set(partitionId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ExecutorServiceSubmitToPartitionCodec::RequestParameters::calculateDataSize(
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
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ExecutorServiceSubmitToPartitionCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.getNullable<serialization::pimpl::Data >();
                }

                ExecutorServiceSubmitToPartitionCodec::ResponseParameters ExecutorServiceSubmitToPartitionCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ExecutorServiceSubmitToPartitionCodec::ResponseParameters(clientMessage);
                }

                ExecutorServiceSubmitToPartitionCodec::ResponseParameters::ResponseParameters(const ExecutorServiceSubmitToPartitionCodec::ResponseParameters &rhs) {
                        response = std::auto_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(*rhs.response));
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

