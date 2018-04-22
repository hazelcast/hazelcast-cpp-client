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

#include "hazelcast/client/protocol/codec/MultiMapValueCountCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapValueCountCodec::REQUEST_TYPE = HZ_MULTIMAP_VALUECOUNT;
                const bool MultiMapValueCountCodec::RETRYABLE = true;
                const ResponseMessageConst MultiMapValueCountCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::auto_ptr<ClientMessage> MultiMapValueCountCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapValueCountCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapValueCountCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MultiMapValueCountCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (RESPONSE_TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException(
                                "MultiMapValueCountCodec::ResponseParameters::decode", clientMessage.getMessageType(),
                                RESPONSE_TYPE);
                    }


                    response = clientMessage.get<int32_t>();

                }

                MultiMapValueCountCodec::ResponseParameters
                MultiMapValueCountCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MultiMapValueCountCodec::ResponseParameters(clientMessage);
                }

                MultiMapValueCountCodec::ResponseParameters::ResponseParameters(
                        const MultiMapValueCountCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }
            }
        }
    }
}

