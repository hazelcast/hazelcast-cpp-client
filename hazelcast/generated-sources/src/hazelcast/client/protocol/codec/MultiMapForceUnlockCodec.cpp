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



#include "hazelcast/client/protocol/codec/MultiMapForceUnlockCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapForceUnlockCodec::RequestParameters::TYPE = HZ_MULTIMAP_FORCEUNLOCK;
                const bool MultiMapForceUnlockCodec::RequestParameters::RETRYABLE = false;
                const int32_t MultiMapForceUnlockCodec::ResponseParameters::TYPE = 100;
                std::auto_ptr<ClientMessage> MultiMapForceUnlockCodec::RequestParameters::encode(
                        const std::string &name, 
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, key);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)MultiMapForceUnlockCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapForceUnlockCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }

                MultiMapForceUnlockCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("MultiMapForceUnlockCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }
                }

                MultiMapForceUnlockCodec::ResponseParameters MultiMapForceUnlockCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MultiMapForceUnlockCodec::ResponseParameters(clientMessage);
                }

                MultiMapForceUnlockCodec::ResponseParameters::ResponseParameters(const MultiMapForceUnlockCodec::ResponseParameters &rhs) {
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

