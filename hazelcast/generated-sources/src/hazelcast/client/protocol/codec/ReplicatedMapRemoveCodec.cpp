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

#include "hazelcast/client/protocol/codec/ReplicatedMapRemoveCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapRemoveCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_REMOVE;
                const bool ReplicatedMapRemoveCodec::RETRYABLE = false;
                const ResponseMessageConst ReplicatedMapRemoveCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> ReplicatedMapRemoveCodec::encodeRequest(
                        const std::string &name, 
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, key);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ReplicatedMapRemoveCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapRemoveCodec::calculateDataSize(
                        const std::string &name, 
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }

                    ReplicatedMapRemoveCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                        
                        

                    response = clientMessage.getNullable<serialization::pimpl::Data >();
                            
                    }

                    ReplicatedMapRemoveCodec::ResponseParameters ReplicatedMapRemoveCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                        return ReplicatedMapRemoveCodec::ResponseParameters(clientMessage);
                    }

                    ReplicatedMapRemoveCodec::ResponseParameters::ResponseParameters(const ReplicatedMapRemoveCodec::ResponseParameters &rhs) {
                            response = std::unique_ptr<serialization::pimpl::Data >(new serialization::pimpl::Data(*rhs.response));
                    }

            }
        }
    }
}

