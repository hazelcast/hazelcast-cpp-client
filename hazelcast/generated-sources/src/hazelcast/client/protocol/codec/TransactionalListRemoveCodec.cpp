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

#include "hazelcast/client/protocol/codec/TransactionalListRemoveCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalListMessageType TransactionalListRemoveCodec::REQUEST_TYPE = HZ_TRANSACTIONALLIST_REMOVE;
                const bool TransactionalListRemoveCodec::RETRYABLE = false;
                const ResponseMessageConst TransactionalListRemoveCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::auto_ptr<ClientMessage> TransactionalListRemoveCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &item) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, item);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalListRemoveCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(item);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalListRemoveCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &item) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(item);
                    return dataSize;
                }

                TransactionalListRemoveCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                TransactionalListRemoveCodec::ResponseParameters
                TransactionalListRemoveCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return TransactionalListRemoveCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

