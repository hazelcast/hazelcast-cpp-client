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

#include "hazelcast/client/protocol/codec/TransactionalMapReplaceIfSameCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapReplaceIfSameCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_REPLACEIFSAME;
                const bool TransactionalMapReplaceIfSameCodec::RETRYABLE = false;
                const ResponseMessageConst TransactionalMapReplaceIfSameCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::auto_ptr<ClientMessage> TransactionalMapReplaceIfSameCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &oldValue,
                        const serialization::pimpl::Data &newValue) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key, oldValue, newValue);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapReplaceIfSameCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->set(oldValue);
                    clientMessage->set(newValue);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapReplaceIfSameCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &oldValue,
                        const serialization::pimpl::Data &newValue) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(oldValue);
                    dataSize += ClientMessage::calculateDataSize(newValue);
                    return dataSize;
                }

                TransactionalMapReplaceIfSameCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {
                    if (RESPONSE_TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException(
                                "TransactionalMapReplaceIfSameCodec::ResponseParameters::decode",
                                clientMessage.getMessageType(), RESPONSE_TYPE);
                    }


                    response = clientMessage.get<bool>();

                }

                TransactionalMapReplaceIfSameCodec::ResponseParameters
                TransactionalMapReplaceIfSameCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return TransactionalMapReplaceIfSameCodec::ResponseParameters(clientMessage);
                }

                TransactionalMapReplaceIfSameCodec::ResponseParameters::ResponseParameters(
                        const TransactionalMapReplaceIfSameCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }
            }
        }
    }
}

