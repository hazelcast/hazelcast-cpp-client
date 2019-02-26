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

#include "hazelcast/client/protocol/codec/ClientGetPartitionsCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientGetPartitionsCodec::REQUEST_TYPE = HZ_CLIENT_GETPARTITIONS;
                const bool ClientGetPartitionsCodec::RETRYABLE = false;
                const ResponseMessageConst ClientGetPartitionsCodec::RESPONSE_TYPE = (ResponseMessageConst) 108;

                std::auto_ptr<ClientMessage> ClientGetPartitionsCodec::encodeRequest() {
                    int32_t requiredDataSize = calculateDataSize();
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientGetPartitionsCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientGetPartitionsCodec::calculateDataSize() {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    return dataSize;
                }

                ClientGetPartitionsCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    partitionStateVersionExist = false;


                    partitions = clientMessage.getArray<std::pair<Address, std::vector<int32_t> > >();
                    if (clientMessage.isComplete()) {
                        return;
                    }

                    partitionStateVersion = clientMessage.get<int32_t>();
                    partitionStateVersionExist = true;
                }

                ClientGetPartitionsCodec::ResponseParameters
                ClientGetPartitionsCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientGetPartitionsCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

