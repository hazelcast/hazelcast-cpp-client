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



#include "hazelcast/client/protocol/codec/MapGetEntryViewCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/map/DataEntryView.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapGetEntryViewCodec::RequestParameters::TYPE = HZ_MAP_GETENTRYVIEW;
                const bool MapGetEntryViewCodec::RequestParameters::RETRYABLE = true;
                const int32_t MapGetEntryViewCodec::ResponseParameters::TYPE = 111;
                std::auto_ptr<ClientMessage> MapGetEntryViewCodec::RequestParameters::encode(
                        const std::string &name, 
                        const serialization::pimpl::Data &key, 
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)MapGetEntryViewCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapGetEntryViewCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const serialization::pimpl::Data &key, 
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MapGetEntryViewCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("MapGetEntryViewCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.getNullable<map::DataEntryView >();
                }

                MapGetEntryViewCodec::ResponseParameters MapGetEntryViewCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return MapGetEntryViewCodec::ResponseParameters(clientMessage);
                }

                MapGetEntryViewCodec::ResponseParameters::ResponseParameters(const MapGetEntryViewCodec::ResponseParameters &rhs) {
                        response = std::auto_ptr<map::DataEntryView>(new map::DataEntryView(*rhs.response));
                }
                //************************ EVENTS END **************************************************************************//

            }
        }
    }
}

