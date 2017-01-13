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



#include "hazelcast/client/protocol/codec/TransactionalQueueSizeCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalQueueMessageType TransactionalQueueSizeCodec::RequestParameters::TYPE = HZ_TRANSACTIONALQUEUE_SIZE;
                const bool TransactionalQueueSizeCodec::RequestParameters::RETRYABLE = false;
                const int32_t TransactionalQueueSizeCodec::ResponseParameters::TYPE = 102;
                std::auto_ptr<ClientMessage> TransactionalQueueSizeCodec::RequestParameters::encode(
                        const std::string &name, 
                        const std::string &txnId, 
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)TransactionalQueueSizeCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalQueueSizeCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const std::string &txnId, 
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                TransactionalQueueSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("TransactionalQueueSizeCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<int32_t >();
                }

                TransactionalQueueSizeCodec::ResponseParameters TransactionalQueueSizeCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return TransactionalQueueSizeCodec::ResponseParameters(clientMessage);
                }

                TransactionalQueueSizeCodec::ResponseParameters::ResponseParameters(const TransactionalQueueSizeCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

