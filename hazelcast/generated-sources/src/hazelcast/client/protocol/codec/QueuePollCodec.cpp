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



#include "hazelcast/client/protocol/codec/QueuePollCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueuePollCodec::RequestParameters::TYPE = HZ_QUEUE_POLL;
                const bool QueuePollCodec::RequestParameters::RETRYABLE = false;
                const int32_t QueuePollCodec::ResponseParameters::TYPE = 105;
                std::auto_ptr<ClientMessage> QueuePollCodec::RequestParameters::encode(
                        const std::string &name, 
                        int64_t timeoutMillis) {
                    int32_t requiredDataSize = calculateDataSize(name, timeoutMillis);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)QueuePollCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(timeoutMillis);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueuePollCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        int64_t timeoutMillis) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(timeoutMillis);
                    return dataSize;
                }

                QueuePollCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("QueuePollCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.getNullable<serialization::pimpl::Data >();
                }

                QueuePollCodec::ResponseParameters QueuePollCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return QueuePollCodec::ResponseParameters(clientMessage);
                }

                QueuePollCodec::ResponseParameters::ResponseParameters(const QueuePollCodec::ResponseParameters &rhs) {
                        response = std::auto_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(*rhs.response));
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

