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

#include "hazelcast/client/protocol/codec/ClientAddPartitionListenerCodec.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientAddPartitionListenerCodec::REQUEST_TYPE = HZ_CLIENT_ADDPARTITIONLISTENER;
                const bool ClientAddPartitionListenerCodec::RETRYABLE = false;
                const ResponseMessageConst ClientAddPartitionListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::auto_ptr<ClientMessage> ClientAddPartitionListenerCodec::encodeRequest() {
                    int32_t requiredDataSize = calculateDataSize();
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientAddPartitionListenerCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientAddPartitionListenerCodec::calculateDataSize() {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    return dataSize;
                }

                ClientAddPartitionListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                }

                ClientAddPartitionListenerCodec::ResponseParameters
                ClientAddPartitionListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientAddPartitionListenerCodec::ResponseParameters(clientMessage);
                }

                ClientAddPartitionListenerCodec::ResponseParameters::ResponseParameters(
                        const ClientAddPartitionListenerCodec::ResponseParameters &rhs) {
                }

                //************************ EVENTS START*************************************************************************//
                ClientAddPartitionListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ClientAddPartitionListenerCodec::AbstractEventHandler::handle(
                        std::auto_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_PARTITIONS: {
                            std::vector<std::pair<Address, std::vector<int32_t> > > partitions = clientMessage->getArray<std::pair<Address, std::vector<int32_t> > >();

                            int32_t partitionStateVersion = clientMessage->get<int32_t>();


                            handlePartitionsEventV15(partitions, partitionStateVersion);
                            break;
                        }
                        default:
                            util::ILogger::getLogger().warning()
                                    << "[ClientAddPartitionListenerCodec::AbstractEventHandler::handle] Unknown message type ("
                                    << messageType << ") received on event handler.";
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

