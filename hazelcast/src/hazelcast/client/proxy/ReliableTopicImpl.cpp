/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/proxy/ReliableTopicImpl.h"
#include "hazelcast/client/topic/impl/TopicEventHandlerImpl.h"
#include "hazelcast/client/ClientConfig.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            ReliableTopicImpl::ReliableTopicImpl(const std::string &instanceName, spi::ClientContext *context,
                                                 std::shared_ptr<Ringbuffer<topic::impl::reliable::ReliableTopicMessage> > rb)
                    : proxy::ProxyImpl("hz:impl:topicService", instanceName, context), ringbuffer(rb),
                      logger(context->getLogger()), config(context->getClientConfig().getReliableTopicConfig(instanceName)) {
            }

            void ReliableTopicImpl::publish(const serialization::pimpl::Data &data) {
                std::unique_ptr<Address> nullAddress;
                topic::impl::reliable::ReliableTopicMessage message(data, nullAddress);
                ringbuffer->add(message);
            }
        }
    }
}

