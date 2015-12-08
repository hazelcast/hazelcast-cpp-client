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



#include "hazelcast/client/protocol/codec/TransactionalQueuePollCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalQueueMessageType TransactionalQueuePollCodec::RequestParameters::TYPE = HZ_TRANSACTIONALQUEUE_POLL;
                const bool TransactionalQueuePollCodec::RequestParameters::RETRYABLE = false;
                const int32_t TransactionalQueuePollCodec::ResponseParameters::TYPE = 105;
                std::auto_ptr<ClientMessage> TransactionalQueuePollCodec::RequestParameters::encode(
                        const std::string &name, 
                        const std::string &txnId, 
                        int64_t threadId, 
                        int64_t timeout) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, timeout);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)TransactionalQueuePollCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(timeout);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalQueuePollCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const std::string &txnId, 
                        int64_t threadId, 
                        int64_t timeout) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(timeout);
                    return dataSize;
                }

                TransactionalQueuePollCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("TransactionalQueuePollCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.getNullable<serialization::pimpl::Data >();
                }

                TransactionalQueuePollCodec::ResponseParameters TransactionalQueuePollCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return TransactionalQueuePollCodec::ResponseParameters(clientMessage);
                }

                TransactionalQueuePollCodec::ResponseParameters::ResponseParameters(const TransactionalQueuePollCodec::ResponseParameters &rhs) {
                        response = std::auto_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(*rhs.response));
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

