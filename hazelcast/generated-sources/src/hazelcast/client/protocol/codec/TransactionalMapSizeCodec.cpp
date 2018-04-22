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

#include "hazelcast/client/protocol/codec/TransactionalMapSizeCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapSizeCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_SIZE;
                const bool TransactionalMapSizeCodec::RETRYABLE = false;
                const ResponseMessageConst TransactionalMapSizeCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::auto_ptr<ClientMessage> TransactionalMapSizeCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapSizeCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapSizeCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                TransactionalMapSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (RESPONSE_TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException(
                                "TransactionalMapSizeCodec::ResponseParameters::decode", clientMessage.getMessageType(),
                                RESPONSE_TYPE);
                    }


                    response = clientMessage.get<int32_t>();

                }

                TransactionalMapSizeCodec::ResponseParameters
                TransactionalMapSizeCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return TransactionalMapSizeCodec::ResponseParameters(clientMessage);
                }

                TransactionalMapSizeCodec::ResponseParameters::ResponseParameters(
                        const TransactionalMapSizeCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }
            }
        }
    }
}

