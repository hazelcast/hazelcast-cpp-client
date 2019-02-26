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

#include "hazelcast/client/protocol/codec/ReplicatedMapPutCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapPutCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_PUT;
                const bool ReplicatedMapPutCodec::RETRYABLE = false;
                const ResponseMessageConst ReplicatedMapPutCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::auto_ptr<ClientMessage> ReplicatedMapPutCodec::encodeRequest(
                        const std::string &name, 
                        const serialization::pimpl::Data &key, 
                        const serialization::pimpl::Data &value, 
                        int64_t ttl) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, ttl);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ReplicatedMapPutCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(ttl);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapPutCodec::calculateDataSize(
                        const std::string &name, 
                        const serialization::pimpl::Data &key, 
                        const serialization::pimpl::Data &value, 
                        int64_t ttl) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(ttl);
                    return dataSize;
                }

                    ReplicatedMapPutCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                        
                        

                    response = clientMessage.getNullable<serialization::pimpl::Data >();
                            
                    }

                    ReplicatedMapPutCodec::ResponseParameters ReplicatedMapPutCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                        return ReplicatedMapPutCodec::ResponseParameters(clientMessage);
                    }

                    ReplicatedMapPutCodec::ResponseParameters::ResponseParameters(const ReplicatedMapPutCodec::ResponseParameters &rhs) {
                            response = std::auto_ptr<serialization::pimpl::Data >(new serialization::pimpl::Data(*rhs.response));
                    }

            }
        }
    }
}

