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

#include "hazelcast/client/protocol/codec/MultiMapRemoveEntryListenerCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapRemoveEntryListenerCodec::REQUEST_TYPE = HZ_MULTIMAP_REMOVEENTRYLISTENER;
                const bool MultiMapRemoveEntryListenerCodec::RETRYABLE = true;
                const ResponseMessageConst MultiMapRemoveEntryListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::auto_ptr<ClientMessage> MultiMapRemoveEntryListenerCodec::encodeRequest(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t requiredDataSize = calculateDataSize(name, registrationId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapRemoveEntryListenerCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(registrationId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapRemoveEntryListenerCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(registrationId);
                    return dataSize;
                }

                MultiMapRemoveEntryListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (RESPONSE_TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException(
                                "MultiMapRemoveEntryListenerCodec::ResponseParameters::decode",
                                clientMessage.getMessageType(), RESPONSE_TYPE);
                    }


                    response = clientMessage.get<bool>();

                }

                MultiMapRemoveEntryListenerCodec::ResponseParameters
                MultiMapRemoveEntryListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MultiMapRemoveEntryListenerCodec::ResponseParameters(clientMessage);
                }

                MultiMapRemoveEntryListenerCodec::ResponseParameters::ResponseParameters(
                        const MultiMapRemoveEntryListenerCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }
            }
        }
    }
}

