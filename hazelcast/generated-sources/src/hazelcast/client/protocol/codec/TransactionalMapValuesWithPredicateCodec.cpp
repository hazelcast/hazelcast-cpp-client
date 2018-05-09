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

#include "hazelcast/client/protocol/codec/TransactionalMapValuesWithPredicateCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapValuesWithPredicateCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_VALUESWITHPREDICATE;
                const bool TransactionalMapValuesWithPredicateCodec::RETRYABLE = false;
                const ResponseMessageConst TransactionalMapValuesWithPredicateCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::auto_ptr<ClientMessage> TransactionalMapValuesWithPredicateCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &predicate) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, predicate);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapValuesWithPredicateCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(predicate);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapValuesWithPredicateCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &predicate) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    return dataSize;
                }

                TransactionalMapValuesWithPredicateCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                TransactionalMapValuesWithPredicateCodec::ResponseParameters
                TransactionalMapValuesWithPredicateCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return TransactionalMapValuesWithPredicateCodec::ResponseParameters(clientMessage);
                }

                TransactionalMapValuesWithPredicateCodec::ResponseParameters::ResponseParameters(
                        const TransactionalMapValuesWithPredicateCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }
            }
        }
    }
}

