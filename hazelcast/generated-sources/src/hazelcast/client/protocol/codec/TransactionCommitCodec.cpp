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

#include "hazelcast/client/protocol/codec/TransactionCommitCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionMessageType TransactionCommitCodec::REQUEST_TYPE = HZ_TRANSACTION_COMMIT;
                const bool TransactionCommitCodec::RETRYABLE = false;
                const ResponseMessageConst TransactionCommitCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::auto_ptr<ClientMessage> TransactionCommitCodec::encodeRequest(
                        const std::string &transactionId,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(transactionId, threadId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionCommitCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(transactionId);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionCommitCodec::calculateDataSize(
                        const std::string &transactionId,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(transactionId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                TransactionCommitCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                }

                TransactionCommitCodec::ResponseParameters
                TransactionCommitCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return TransactionCommitCodec::ResponseParameters(clientMessage);
                }

                TransactionCommitCodec::ResponseParameters::ResponseParameters(
                        const TransactionCommitCodec::ResponseParameters &rhs) {
                }
            }
        }
    }
}

