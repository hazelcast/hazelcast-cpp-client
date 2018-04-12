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



#include "hazelcast/client/protocol/codec/ClientGetPartitionsCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientGetPartitionsCodec::RequestParameters::TYPE = HZ_CLIENT_GETPARTITIONS;
                const bool ClientGetPartitionsCodec::RequestParameters::RETRYABLE = false;
                const int32_t ClientGetPartitionsCodec::ResponseParameters::TYPE = 108;
                std::auto_ptr<ClientMessage> ClientGetPartitionsCodec::RequestParameters::encode() {
                    int32_t requiredDataSize = calculateDataSize();
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ClientGetPartitionsCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientGetPartitionsCodec::RequestParameters::calculateDataSize() {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    return dataSize;
                }

                ClientGetPartitionsCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ClientGetPartitionsCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    partitions = clientMessage.getArray<std::pair<Address, std::vector<int32_t> > >();

                    partitionStateVersion = clientMessage.get<int32_t >();
                }

                ClientGetPartitionsCodec::ResponseParameters ClientGetPartitionsCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientGetPartitionsCodec::ResponseParameters(clientMessage);
                }

                ClientGetPartitionsCodec::ResponseParameters::ResponseParameters(const ClientGetPartitionsCodec::ResponseParameters &rhs) {
                        partitions = rhs.partitions;
                        partitionStateVersion = rhs.partitionStateVersion;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

