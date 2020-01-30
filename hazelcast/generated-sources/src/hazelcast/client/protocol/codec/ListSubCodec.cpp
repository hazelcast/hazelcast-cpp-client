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

#include "hazelcast/client/protocol/codec/ListSubCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListSubCodec::REQUEST_TYPE = HZ_LIST_SUB;
                const bool ListSubCodec::RETRYABLE = true;
                const ResponseMessageConst ListSubCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> ListSubCodec::encodeRequest(
                        const std::string &name,
                        int32_t from,
                        int32_t to) {
                    int32_t requiredDataSize = calculateDataSize(name, from, to);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListSubCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(from);
                    clientMessage->set(to);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListSubCodec::calculateDataSize(
                        const std::string &name,
                        int32_t from,
                        int32_t to) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(from);
                    dataSize += ClientMessage::calculateDataSize(to);
                    return dataSize;
                }

                ListSubCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                ListSubCodec::ResponseParameters
                ListSubCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ListSubCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

