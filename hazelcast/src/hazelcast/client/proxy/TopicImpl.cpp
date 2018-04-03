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
//
// Created by sancar koyunlu on 01/10/14.
//

#include "hazelcast/client/proxy/ITopicImpl.h"

#include "hazelcast/client/topic/impl/TopicEventHandlerImpl.h"
#include "hazelcast/client/spi/ClientListenerService.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/TopicPublishCodec.h"
#include "hazelcast/client/protocol/codec/TopicAddMessageListenerCodec.h"
#include "hazelcast/client/protocol/codec/TopicRemoveMessageListenerCodec.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            ITopicImpl::ITopicImpl(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::ProxyImpl("hz:impl:topicService", instanceName, context) {
                partitionId = getPartitionId(toData(instanceName));
            }

            void ITopicImpl::publish(const serialization::pimpl::Data &data) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TopicPublishCodec::RequestParameters::encode(getName(), data);

                invokeOnPartition(request, partitionId);
            }

            std::string ITopicImpl::addMessageListener(impl::BaseEventHandler *topicEventHandler) {
                return registerListener(createItemListenerCodec(), topicEventHandler);
            }

            bool ITopicImpl::removeMessageListener(const std::string &registrationId) {
                return context->getClientListenerService().deregisterListener(registrationId);
            }

            boost::shared_ptr<spi::impl::ListenerMessageCodec> ITopicImpl::createItemListenerCodec() {
                return boost::shared_ptr<spi::impl::ListenerMessageCodec>(new TopicListenerMessageCodec(getName()));
            }

            ITopicImpl::TopicListenerMessageCodec::TopicListenerMessageCodec(const std::string &name) : name(name) {}

            std::auto_ptr<protocol::ClientMessage>
            ITopicImpl::TopicListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::TopicAddMessageListenerCodec(name, localOnly).encodeRequest();
            }

            std::string ITopicImpl::TopicListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::TopicAddMessageListenerCodec(name, false).decodeResponse(responseMessage);
            }

            std::auto_ptr<protocol::ClientMessage>
            ITopicImpl::TopicListenerMessageCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::TopicRemoveMessageListenerCodec(name, realRegistrationId).encodeRequest();
            }

            bool ITopicImpl::TopicListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::TopicRemoveMessageListenerCodec(name, "").decodeResponse(clientMessage);
            }

        }
    }
}

