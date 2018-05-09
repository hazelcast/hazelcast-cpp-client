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

#include "hazelcast/client/protocol/codec/QueueDrainToCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueDrainToCodec::REQUEST_TYPE = HZ_QUEUE_DRAINTO;
                const bool QueueDrainToCodec::RETRYABLE = false;
                const ResponseMessageConst QueueDrainToCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::auto_ptr<ClientMessage> QueueDrainToCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueDrainToCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueDrainToCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                QueueDrainToCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                QueueDrainToCodec::ResponseParameters
                QueueDrainToCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return QueueDrainToCodec::ResponseParameters(clientMessage);
                }

                QueueDrainToCodec::ResponseParameters::ResponseParameters(
                        const QueueDrainToCodec::ResponseParameters &rhs) {
                    response = rhs.response;
                }
            }
        }
    }
}

