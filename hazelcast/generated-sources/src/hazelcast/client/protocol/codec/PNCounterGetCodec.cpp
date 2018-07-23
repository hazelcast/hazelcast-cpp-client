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

#include "hazelcast/client/protocol/codec/PNCounterGetCodec.h"
#include "hazelcast/client/Address.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const PNCounterMessageType PNCounterGetCodec::REQUEST_TYPE = HZ_PNCOUNTER_GET;
                const bool PNCounterGetCodec::RETRYABLE = true;
                const ResponseMessageConst PNCounterGetCodec::RESPONSE_TYPE = (ResponseMessageConst) 127;

                std::auto_ptr<ClientMessage> PNCounterGetCodec::encodeRequest(
                        const std::string &name, 
                        const std::vector<std::pair<std::string, int64_t > > &replicaTimestamps, 
                        const Address &targetReplica) {
                    int32_t requiredDataSize = calculateDataSize(name, replicaTimestamps, targetReplica);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)PNCounterGetCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<std::pair<std::string, int64_t > >(replicaTimestamps);
                    clientMessage->set(targetReplica);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t PNCounterGetCodec::calculateDataSize(
                        const std::string &name, 
                        const std::vector<std::pair<std::string, int64_t > > &replicaTimestamps, 
                        const Address &targetReplica) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<std::pair<std::string, int64_t > >(replicaTimestamps);
                    dataSize += ClientMessage::calculateDataSize(targetReplica);
                    return dataSize;
                }

                    PNCounterGetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                        
                        

                    value = clientMessage.get<int64_t >();
                            

                    replicaTimestamps = clientMessage.getArray<std::pair<std::string, int64_t > >();
                            

                    replicaCount = clientMessage.get<int32_t >();
                            
                    }

                    PNCounterGetCodec::ResponseParameters PNCounterGetCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                        return PNCounterGetCodec::ResponseParameters(clientMessage);
                    }


            }
        }
    }
}

