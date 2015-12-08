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



#include "hazelcast/client/protocol/codec/AtomicReferenceContainsCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const AtomicReferenceMessageType AtomicReferenceContainsCodec::RequestParameters::TYPE = HZ_ATOMICREFERENCE_CONTAINS;
                const bool AtomicReferenceContainsCodec::RequestParameters::RETRYABLE = true;
                const int32_t AtomicReferenceContainsCodec::ResponseParameters::TYPE = 101;
                std::auto_ptr<ClientMessage> AtomicReferenceContainsCodec::RequestParameters::encode(
                        const std::string &name, 
                        const serialization::pimpl::Data *expected) {
                    int32_t requiredDataSize = calculateDataSize(name, expected);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)AtomicReferenceContainsCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(expected);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t AtomicReferenceContainsCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const serialization::pimpl::Data *expected) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(expected);
                    return dataSize;
                }

                AtomicReferenceContainsCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("AtomicReferenceContainsCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<bool >();
                }

                AtomicReferenceContainsCodec::ResponseParameters AtomicReferenceContainsCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return AtomicReferenceContainsCodec::ResponseParameters(clientMessage);
                }

                AtomicReferenceContainsCodec::ResponseParameters::ResponseParameters(const AtomicReferenceContainsCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

