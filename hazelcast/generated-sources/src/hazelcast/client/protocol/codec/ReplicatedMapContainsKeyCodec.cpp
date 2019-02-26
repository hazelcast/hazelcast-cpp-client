/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/protocol/codec/ReplicatedMapContainsKeyCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapContainsKeyCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_CONTAINSKEY;
                const bool ReplicatedMapContainsKeyCodec::RETRYABLE = true;
                const ResponseMessageConst ReplicatedMapContainsKeyCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::auto_ptr<ClientMessage> ReplicatedMapContainsKeyCodec::encodeRequest(
                        const std::string &name, 
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, key);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ReplicatedMapContainsKeyCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapContainsKeyCodec::calculateDataSize(
                        const std::string &name, 
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }

                    ReplicatedMapContainsKeyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                        
                        

                    response = clientMessage.get<bool >();
                            
                    }

                    ReplicatedMapContainsKeyCodec::ResponseParameters ReplicatedMapContainsKeyCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                        return ReplicatedMapContainsKeyCodec::ResponseParameters(clientMessage);
                    }


            }
        }
    }
}

