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

#include "hazelcast/client/protocol/codec/TransactionalQueuePollCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalQueueMessageType TransactionalQueuePollCodec::REQUEST_TYPE = HZ_TRANSACTIONALQUEUE_POLL;
                const bool TransactionalQueuePollCodec::RETRYABLE = false;
                const ResponseMessageConst TransactionalQueuePollCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::auto_ptr<ClientMessage> TransactionalQueuePollCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        int64_t timeout) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, timeout);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalQueuePollCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(timeout);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalQueuePollCodec::calculateDataSize(
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


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                TransactionalQueuePollCodec::ResponseParameters
                TransactionalQueuePollCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return TransactionalQueuePollCodec::ResponseParameters(clientMessage);
                }

                TransactionalQueuePollCodec::ResponseParameters::ResponseParameters(
                        const TransactionalQueuePollCodec::ResponseParameters &rhs) {
                    response = std::auto_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(*rhs.response));
                }
            }
        }
    }
}

