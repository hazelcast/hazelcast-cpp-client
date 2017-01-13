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



#include "hazelcast/client/protocol/codec/ListIndexOfCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListIndexOfCodec::RequestParameters::TYPE = HZ_LIST_INDEXOF;
                const bool ListIndexOfCodec::RequestParameters::RETRYABLE = true;
                const int32_t ListIndexOfCodec::ResponseParameters::TYPE = 102;
                std::auto_ptr<ClientMessage> ListIndexOfCodec::RequestParameters::encode(
                        const std::string &name, 
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ListIndexOfCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListIndexOfCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                ListIndexOfCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ListIndexOfCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<int32_t >();
                }

                ListIndexOfCodec::ResponseParameters ListIndexOfCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ListIndexOfCodec::ResponseParameters(clientMessage);
                }

                ListIndexOfCodec::ResponseParameters::ResponseParameters(const ListIndexOfCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

