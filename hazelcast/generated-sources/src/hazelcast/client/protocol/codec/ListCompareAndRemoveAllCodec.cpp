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

#include "hazelcast/client/protocol/codec/ListCompareAndRemoveAllCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListCompareAndRemoveAllCodec::REQUEST_TYPE = HZ_LIST_COMPAREANDREMOVEALL;
                const bool ListCompareAndRemoveAllCodec::RETRYABLE = false;
                const ResponseMessageConst ListCompareAndRemoveAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::auto_ptr<ClientMessage> ListCompareAndRemoveAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &values) {
                    int32_t requiredDataSize = calculateDataSize(name, values);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListCompareAndRemoveAllCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(values);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListCompareAndRemoveAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &values) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(values);
                    return dataSize;
                }

                ListCompareAndRemoveAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ListCompareAndRemoveAllCodec::ResponseParameters
                ListCompareAndRemoveAllCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ListCompareAndRemoveAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

