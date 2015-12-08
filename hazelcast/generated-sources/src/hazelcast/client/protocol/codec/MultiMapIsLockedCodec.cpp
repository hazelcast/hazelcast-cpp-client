/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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



#include "hazelcast/client/protocol/codec/MultiMapIsLockedCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapIsLockedCodec::RequestParameters::TYPE = HZ_MULTIMAP_ISLOCKED;
                const bool MultiMapIsLockedCodec::RequestParameters::RETRYABLE = true;
                const int32_t MultiMapIsLockedCodec::ResponseParameters::TYPE = 101;
                std::auto_ptr<ClientMessage> MultiMapIsLockedCodec::RequestParameters::encode(
                        const std::string &name, 
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, key);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)MultiMapIsLockedCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapIsLockedCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }

                MultiMapIsLockedCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("MultiMapIsLockedCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<bool >();
                }

                MultiMapIsLockedCodec::ResponseParameters MultiMapIsLockedCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MultiMapIsLockedCodec::ResponseParameters(clientMessage);
                }

                MultiMapIsLockedCodec::ResponseParameters::ResponseParameters(const MultiMapIsLockedCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

