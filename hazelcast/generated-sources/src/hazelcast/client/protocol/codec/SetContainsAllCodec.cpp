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



#include "hazelcast/client/protocol/codec/SetContainsAllCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetContainsAllCodec::RequestParameters::TYPE = HZ_SET_CONTAINSALL;
                const bool SetContainsAllCodec::RequestParameters::RETRYABLE = false;
                const int32_t SetContainsAllCodec::ResponseParameters::TYPE = 101;
                std::auto_ptr<ClientMessage> SetContainsAllCodec::RequestParameters::encode(
                        const std::string &name, 
                        const std::vector<serialization::pimpl::Data > &items) {
                    int32_t requiredDataSize = calculateDataSize(name, items);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)SetContainsAllCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data >(items);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetContainsAllCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const std::vector<serialization::pimpl::Data > &items) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data >(items);
                    return dataSize;
                }

                SetContainsAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("SetContainsAllCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<bool >();
                }

                SetContainsAllCodec::ResponseParameters SetContainsAllCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return SetContainsAllCodec::ResponseParameters(clientMessage);
                }

                SetContainsAllCodec::ResponseParameters::ResponseParameters(const SetContainsAllCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

