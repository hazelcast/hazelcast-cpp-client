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

#include "hazelcast/util/Util.h"
#include "hazelcast/util/ILogger.h"

#include "hazelcast/client/protocol/codec/ListRemoveListenerCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListRemoveListenerCodec::RequestParameters::TYPE = HZ_LIST_REMOVELISTENER;
                const bool ListRemoveListenerCodec::RequestParameters::RETRYABLE = true;
                const int32_t ListRemoveListenerCodec::ResponseParameters::TYPE = 101;

                ListRemoveListenerCodec::~ListRemoveListenerCodec() {
                }

                std::auto_ptr<ClientMessage> ListRemoveListenerCodec::RequestParameters::encode(
                        const std::string &name, 
                        const std::string &registrationId) {
                    int32_t requiredDataSize = calculateDataSize(name, registrationId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)ListRemoveListenerCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(registrationId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListRemoveListenerCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const std::string &registrationId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(registrationId);
                    return dataSize;
                }

                ListRemoveListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("ListRemoveListenerCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<bool >();
                }

                ListRemoveListenerCodec::ResponseParameters ListRemoveListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ListRemoveListenerCodec::ResponseParameters(clientMessage);
                }

                ListRemoveListenerCodec::ResponseParameters::ResponseParameters(const ListRemoveListenerCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

                ListRemoveListenerCodec::ListRemoveListenerCodec (const std::string &name, const std::string &registrationId)
                        : name_(name), registrationId_(registrationId) {
                }

                //************************ IRemoveListenerCodec interface start ************************************************//
                std::auto_ptr<ClientMessage> ListRemoveListenerCodec::encodeRequest() const {
                    return RequestParameters::encode(name_, registrationId_);
                }

                const std::string &ListRemoveListenerCodec::getRegistrationId() const {
                    return registrationId_;
                }

                void ListRemoveListenerCodec::setRegistrationId(const std::string &id) {
                    registrationId_ = id;
                }

                bool ListRemoveListenerCodec::decodeResponse(ClientMessage &responseMessage) const {
                    return ResponseParameters::decode(responseMessage).response;
                }
                //************************ IRemoveListenerCodec interface ends *************************************************//

            }
        }
    }
}

