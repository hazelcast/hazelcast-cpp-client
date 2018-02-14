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



#include "hazelcast/client/protocol/codec/ListLastIndexOfCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListLastIndexOfCodec::RequestParameters::TYPE = HZ_LIST_LASTINDEXOF;
                const bool ListLastIndexOfCodec::RequestParameters::RETRYABLE = true;
                const int32_t ListLastIndexOfCodec::ResponseParameters::TYPE = 102;
                std::auto_ptr<ClientMessage> ListLastIndexOfCodec::RequestParameters::encode(
                        const std::string &name, 
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ListLastIndexOfCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListLastIndexOfCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                ListLastIndexOfCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ListLastIndexOfCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<int32_t >();
                }

                ListLastIndexOfCodec::ResponseParameters ListLastIndexOfCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ListLastIndexOfCodec::ResponseParameters(clientMessage);
                }

                ListLastIndexOfCodec::ResponseParameters::ResponseParameters(const ListLastIndexOfCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

