/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/topic/PublishRequest.h"
#include "hazelcast/client/topic/AddMessageListenerRequest.h"
#include "hazelcast/client/topic/RemoveMessageListenerRequest.h"
#include "hazelcast/client/topic/TopicEventHandler.h"
#include "hazelcast/client/spi/ServerListenerService.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            ITopicImpl::ITopicImpl(const std::string& instanceName, spi::ClientContext *context)
            : proxy::ProxyImpl("hz:impl:topicService", instanceName, context) {
                partitionId = getPartitionId(toData(instanceName));
            }

            void ITopicImpl::publish(const serialization::pimpl::Data& data) {
                topic::PublishRequest *request = new topic::PublishRequest(getName(), data);
                invoke(request, partitionId);
            }


            std::string ITopicImpl::addMessageListener(impl::BaseEventHandler *topicEventHandler) {
                topic::AddMessageListenerRequest *request = new topic::AddMessageListenerRequest(getName());
                return listen(request, partitionId, topicEventHandler);
            }

            bool ITopicImpl::removeMessageListener(const std::string& registrationId) {
                topic::RemoveMessageListenerRequest *request = new topic::RemoveMessageListenerRequest(getName(), registrationId);
                return stopListening(request, registrationId);
            }
        }
    }
}

