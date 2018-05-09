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

#include "hazelcast/client/protocol/codec/TransactionCreateCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionMessageType TransactionCreateCodec::REQUEST_TYPE = HZ_TRANSACTION_CREATE;
                const bool TransactionCreateCodec::RETRYABLE = false;
                const ResponseMessageConst TransactionCreateCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::auto_ptr<ClientMessage> TransactionCreateCodec::encodeRequest(
                        int64_t timeout,
                        int32_t durability,
                        int32_t transactionType,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(timeout, durability, transactionType, threadId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionCreateCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(timeout);
                    clientMessage->set(durability);
                    clientMessage->set(transactionType);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionCreateCodec::calculateDataSize(
                        int64_t timeout,
                        int32_t durability,
                        int32_t transactionType,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(timeout);
                    dataSize += ClientMessage::calculateDataSize(durability);
                    dataSize += ClientMessage::calculateDataSize(transactionType);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                TransactionCreateCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                TransactionCreateCodec::ResponseParameters
                TransactionCreateCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return TransactionCreateCodec::ResponseParameters(clientMessage);
                }

                TransactionCreateCodec::ResponseParameters::ResponseParameters(
                        const TransactionCreateCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }
            }
        }
    }
}

