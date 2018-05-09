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

#include "hazelcast/client/protocol/codec/ListAddWithIndexCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListAddWithIndexCodec::REQUEST_TYPE = HZ_LIST_ADDWITHINDEX;
                const bool ListAddWithIndexCodec::RETRYABLE = false;
                const ResponseMessageConst ListAddWithIndexCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::auto_ptr<ClientMessage> ListAddWithIndexCodec::encodeRequest(
                        const std::string &name,
                        int32_t index,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, index, value);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListAddWithIndexCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(index);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListAddWithIndexCodec::calculateDataSize(
                        const std::string &name,
                        int32_t index,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(index);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                ListAddWithIndexCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                }

                ListAddWithIndexCodec::ResponseParameters
                ListAddWithIndexCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ListAddWithIndexCodec::ResponseParameters(clientMessage);
                }

                ListAddWithIndexCodec::ResponseParameters::ResponseParameters(
                        const ListAddWithIndexCodec::ResponseParameters &rhs) {
                }
            }
        }
    }
}

