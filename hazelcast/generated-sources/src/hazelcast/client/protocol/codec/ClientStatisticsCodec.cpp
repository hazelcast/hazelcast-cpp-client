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

#include "hazelcast/client/protocol/codec/ClientStatisticsCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientStatisticsCodec::REQUEST_TYPE = HZ_CLIENT_STATISTICS;
                const bool ClientStatisticsCodec::RETRYABLE = false;
                const ResponseMessageConst ClientStatisticsCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> ClientStatisticsCodec::encodeRequest(
                        const std::string &stats) {
                    int32_t requiredDataSize = calculateDataSize(stats);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientStatisticsCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(stats);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientStatisticsCodec::calculateDataSize(
                        const std::string &stats) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(stats);
                    return dataSize;
                }


            }
        }
    }
}

