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

#include "hazelcast/client/protocol/codec/PNCounterAddCodec.h"
#include "hazelcast/client/Address.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const PNCounterMessageType PNCounterAddCodec::REQUEST_TYPE = HZ_PNCOUNTER_ADD;
                const bool PNCounterAddCodec::RETRYABLE = false;
                const ResponseMessageConst PNCounterAddCodec::RESPONSE_TYPE = (ResponseMessageConst) 127;

                std::auto_ptr<ClientMessage> PNCounterAddCodec::encodeRequest(
                        const std::string &name, 
                        int64_t delta, 
                        bool getBeforeUpdate, 
                        const std::vector<std::pair<std::string, int64_t > > &replicaTimestamps, 
                        const Address &targetReplica) {
                    int32_t requiredDataSize = calculateDataSize(name, delta, getBeforeUpdate, replicaTimestamps, targetReplica);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)PNCounterAddCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(delta);
                    clientMessage->set(getBeforeUpdate);
                    clientMessage->setArray<std::pair<std::string, int64_t > >(replicaTimestamps);
                    clientMessage->set(targetReplica);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t PNCounterAddCodec::calculateDataSize(
                        const std::string &name, 
                        int64_t delta, 
                        bool getBeforeUpdate, 
                        const std::vector<std::pair<std::string, int64_t > > &replicaTimestamps, 
                        const Address &targetReplica) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(delta);
                    dataSize += ClientMessage::calculateDataSize(getBeforeUpdate);
                    dataSize += ClientMessage::calculateDataSize<std::pair<std::string, int64_t > >(replicaTimestamps);
                    dataSize += ClientMessage::calculateDataSize(targetReplica);
                    return dataSize;
                }

                    PNCounterAddCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                        
                        

                    value = clientMessage.get<int64_t >();
                            

                    replicaTimestamps = clientMessage.getArray<std::pair<std::string, int64_t > >();
                            

                    replicaCount = clientMessage.get<int32_t >();
                            
                    }

                    PNCounterAddCodec::ResponseParameters PNCounterAddCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                        return PNCounterAddCodec::ResponseParameters(clientMessage);
                    }


            }
        }
    }
}

