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

#include "hazelcast/client/topic/impl/TopicEventHandlerImpl.h"
#include "hazelcast/client/mixedtype/ITopic.h"

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            std::string ITopic::addMessageListener(topic::MessageListener &listener) {
                client::impl::BaseEventHandler *topicEventHandler = new mixedtype::topic::impl::TopicEventHandlerImpl(
                        getName(), getContext().getClientClusterService(),
                        getContext().getSerializationService(),
                        listener);
                return proxy::ITopicImpl::addMessageListener(topicEventHandler);
            }

            bool ITopic::removeMessageListener(const std::string &registrationId) {
                return proxy::ITopicImpl::removeMessageListener(registrationId);
            }

            ITopic::ITopic(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::ITopicImpl(instanceName, context) {
            }
        }
    }
}
