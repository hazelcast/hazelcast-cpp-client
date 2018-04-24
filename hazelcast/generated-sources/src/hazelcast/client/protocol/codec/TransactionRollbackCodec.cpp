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

#include "hazelcast/client/protocol/codec/TransactionRollbackCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionMessageType TransactionRollbackCodec::REQUEST_TYPE = HZ_TRANSACTION_ROLLBACK;
                const bool TransactionRollbackCodec::RETRYABLE = false;
                const ResponseMessageConst TransactionRollbackCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::auto_ptr<ClientMessage> TransactionRollbackCodec::encodeRequest(
                        const std::string &transactionId,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(transactionId, threadId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionRollbackCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(transactionId);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionRollbackCodec::calculateDataSize(
                        const std::string &transactionId,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(transactionId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                TransactionRollbackCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (RESPONSE_TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException(
                                "TransactionRollbackCodec::ResponseParameters::decode", clientMessage.getMessageType(),
                                RESPONSE_TYPE);
                    }


                }

                TransactionRollbackCodec::ResponseParameters
                TransactionRollbackCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return TransactionRollbackCodec::ResponseParameters(clientMessage);
                }

                TransactionRollbackCodec::ResponseParameters::ResponseParameters(
                        const TransactionRollbackCodec::ResponseParameters &rhs) {
                }
            }
        }
    }
}

