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

#include "hazelcast/client/protocol/codec/ListContainsCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListContainsCodec::REQUEST_TYPE = HZ_LIST_CONTAINS;
                const bool ListContainsCodec::RETRYABLE = true;
                const ResponseMessageConst ListContainsCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::auto_ptr<ClientMessage> ListContainsCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListContainsCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListContainsCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                ListContainsCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (RESPONSE_TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ListContainsCodec::ResponseParameters::decode",
                                                                        clientMessage.getMessageType(), RESPONSE_TYPE);
                    }


                    response = clientMessage.get<bool>();

                }

                ListContainsCodec::ResponseParameters
                ListContainsCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ListContainsCodec::ResponseParameters(clientMessage);
                }

                ListContainsCodec::ResponseParameters::ResponseParameters(
                        const ListContainsCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }
            }
        }
    }
}

