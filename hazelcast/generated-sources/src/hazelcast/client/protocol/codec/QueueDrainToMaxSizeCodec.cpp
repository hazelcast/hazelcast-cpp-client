/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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



#include "hazelcast/client/protocol/codec/QueueDrainToMaxSizeCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueDrainToMaxSizeCodec::RequestParameters::TYPE = HZ_QUEUE_DRAINTOMAXSIZE;
                const bool QueueDrainToMaxSizeCodec::RequestParameters::RETRYABLE = false;
                const int32_t QueueDrainToMaxSizeCodec::ResponseParameters::TYPE = 106;
                std::auto_ptr<ClientMessage> QueueDrainToMaxSizeCodec::RequestParameters::encode(
                        const std::string &name, 
                        int32_t maxSize) {
                    int32_t requiredDataSize = calculateDataSize(name, maxSize);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)QueueDrainToMaxSizeCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(maxSize);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueDrainToMaxSizeCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        int32_t maxSize) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(maxSize);
                    return dataSize;
                }

                QueueDrainToMaxSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("QueueDrainToMaxSizeCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.getArray<serialization::pimpl::Data >();
                }

                QueueDrainToMaxSizeCodec::ResponseParameters QueueDrainToMaxSizeCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return QueueDrainToMaxSizeCodec::ResponseParameters(clientMessage);
                }

                QueueDrainToMaxSizeCodec::ResponseParameters::ResponseParameters(const QueueDrainToMaxSizeCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

