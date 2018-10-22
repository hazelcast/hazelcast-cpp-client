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

#include "hazelcast/client/protocol/codec/ReplicatedMapPutAllCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapPutAllCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_PUTALL;
                const bool ReplicatedMapPutAllCodec::RETRYABLE = false;
                const ResponseMessageConst ReplicatedMapPutAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::auto_ptr<ClientMessage> ReplicatedMapPutAllCodec::encodeRequest(
                        const std::string &name, 
                        const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > > &entries) {
                    int32_t requiredDataSize = calculateDataSize(name, entries);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ReplicatedMapPutAllCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > >(entries);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapPutAllCodec::calculateDataSize(
                        const std::string &name, 
                        const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > > &entries) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > >(entries);
                    return dataSize;
                }


            }
        }
    }
}

