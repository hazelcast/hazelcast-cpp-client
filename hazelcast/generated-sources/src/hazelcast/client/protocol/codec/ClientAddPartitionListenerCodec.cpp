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



#include <hazelcast/util/Util.h>
#include <hazelcast/util/ILogger.h>
#include "hazelcast/client/protocol/codec/ClientAddPartitionListenerCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientAddPartitionListenerCodec::RequestParameters::TYPE = HZ_CLIENT_ADDPARTITIONLISTENER;
                const bool ClientAddPartitionListenerCodec::RequestParameters::RETRYABLE = false;
                const int32_t ClientAddPartitionListenerCodec::ResponseParameters::TYPE = 100;
                std::auto_ptr<ClientMessage> ClientAddPartitionListenerCodec::RequestParameters::encode() {
                    int32_t requiredDataSize = calculateDataSize();
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ClientAddPartitionListenerCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientAddPartitionListenerCodec::RequestParameters::calculateDataSize() {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    return dataSize;
                }

                ClientAddPartitionListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ClientAddPartitionListenerCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }
                }

                ClientAddPartitionListenerCodec::ResponseParameters ClientAddPartitionListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientAddPartitionListenerCodec::ResponseParameters(clientMessage);
                }

                ClientAddPartitionListenerCodec::ResponseParameters::ResponseParameters(const ClientAddPartitionListenerCodec::ResponseParameters &rhs) {
                }

                //************************ EVENTS START*************************************************************************//
                ClientAddPartitionListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ClientAddPartitionListenerCodec::AbstractEventHandler::handle(std::auto_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_PARTITIONS:
                        {
                            std::vector<std::pair<Address, std::vector<int32_t> > > partitions = clientMessage->getArray<std::pair<Address, std::vector<int32_t> > >();
                            
                            int32_t partitionStateVersion = clientMessage->get<int32_t >();
                            
                            handlePartitions(partitions, partitionStateVersion);
                            break;
                        }
                        default:
                            char buf[300];
                            util::hz_snprintf(buf, 300, "[ClientAddPartitionListenerCodec::AbstractEventHandler::handle] Unknown message type (%d) received on event handler.", clientMessage->getMessageType());
                            util::ILogger::getLogger().warning(buf);
                    }
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

