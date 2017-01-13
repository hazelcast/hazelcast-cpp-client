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



#include "hazelcast/client/protocol/codec/TransactionalMapPutCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapPutCodec::RequestParameters::TYPE = HZ_TRANSACTIONALMAP_PUT;
                const bool TransactionalMapPutCodec::RequestParameters::RETRYABLE = false;
                const int32_t TransactionalMapPutCodec::ResponseParameters::TYPE = 105;
                std::auto_ptr<ClientMessage> TransactionalMapPutCodec::RequestParameters::encode(
                        const std::string &name, 
                        const std::string &txnId, 
                        int64_t threadId, 
                        const serialization::pimpl::Data &key, 
                        const serialization::pimpl::Data &value, 
                        int64_t ttl) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key, value, ttl);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)TransactionalMapPutCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(ttl);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapPutCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const std::string &txnId, 
                        int64_t threadId, 
                        const serialization::pimpl::Data &key, 
                        const serialization::pimpl::Data &value, 
                        int64_t ttl) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(ttl);
                    return dataSize;
                }

                TransactionalMapPutCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("TransactionalMapPutCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.getNullable<serialization::pimpl::Data >();
                }

                TransactionalMapPutCodec::ResponseParameters TransactionalMapPutCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return TransactionalMapPutCodec::ResponseParameters(clientMessage);
                }

                TransactionalMapPutCodec::ResponseParameters::ResponseParameters(const TransactionalMapPutCodec::ResponseParameters &rhs) {
                        response = std::auto_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(*rhs.response));
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

