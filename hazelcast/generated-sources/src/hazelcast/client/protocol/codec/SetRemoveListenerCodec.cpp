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

#include "hazelcast/client/protocol/codec/SetRemoveListenerCodec.h"
#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetRemoveListenerCodec::RequestParameters::TYPE = HZ_SET_REMOVELISTENER;
                const bool SetRemoveListenerCodec::RequestParameters::RETRYABLE = true;
                const int32_t SetRemoveListenerCodec::ResponseParameters::TYPE = 101;

                SetRemoveListenerCodec::~SetRemoveListenerCodec() {
                }

                std::auto_ptr<ClientMessage> SetRemoveListenerCodec::RequestParameters::encode(
                        const std::string &name, 
                        const std::string &registrationId) {
                    int32_t requiredDataSize = calculateDataSize(name, registrationId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)SetRemoveListenerCodec::RequestParameters::TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(registrationId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetRemoveListenerCodec::RequestParameters::calculateDataSize(
                        const std::string &name, 
                        const std::string &registrationId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(registrationId);
                    return dataSize;
                }

                SetRemoveListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    if (TYPE != clientMessage.getMessageType()) {
                        throw exception::UnexpectedMessageTypeException("SetRemoveListenerCodec::ResponseParameters::decode", clientMessage.getMessageType(), TYPE);
                    }

                    response = clientMessage.get<bool >();
                }

                SetRemoveListenerCodec::ResponseParameters SetRemoveListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return SetRemoveListenerCodec::ResponseParameters(clientMessage);
                }

                SetRemoveListenerCodec::ResponseParameters::ResponseParameters(const SetRemoveListenerCodec::ResponseParameters &rhs) {
                        response = rhs.response;
                }
                //************************ EVENTS END **************************************************************************//

                SetRemoveListenerCodec::SetRemoveListenerCodec (const std::string &name, const std::string &registrationId)
                        : name_(name), registrationId_(registrationId) {
                }

                //************************ IRemoveListenerCodec interface start ************************************************//
                std::auto_ptr<ClientMessage> SetRemoveListenerCodec::encodeRequest() const {
                    return RequestParameters::encode(name_, registrationId_);
                }

                const std::string &SetRemoveListenerCodec::getRegistrationId() const {
                    return registrationId_;
                }

                void SetRemoveListenerCodec::setRegistrationId(const std::string &id) {
                    registrationId_ = id;
                }

                bool SetRemoveListenerCodec::decodeResponse(ClientMessage &responseMessage) const {
                    return ResponseParameters::decode(responseMessage).response;
                }
                //************************ IRemoveListenerCodec interface ends *************************************************//

            }
        }
    }
}

