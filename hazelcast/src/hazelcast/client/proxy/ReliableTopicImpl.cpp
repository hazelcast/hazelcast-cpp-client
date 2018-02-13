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

#include "hazelcast/util/Util.h"
#include "hazelcast/client/proxy/ReliableTopicImpl.h"
#include "hazelcast/client/topic/impl/TopicEventHandlerImpl.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/connection/Connection.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            ReliableTopicImpl::ReliableTopicImpl(const std::string &instanceName, spi::ClientContext *context,
                                                 boost::shared_ptr<Ringbuffer<topic::impl::reliable::ReliableTopicMessage> > rb)
                    : proxy::ProxyImpl("hz:impl:topicService", instanceName, context), ringbuffer(rb),
                      logger(util::ILogger::getLogger()), config(context->getClientConfig().getReliableTopicConfig(instanceName)) {
            }

            void ReliableTopicImpl::publish(const serialization::pimpl::Data &data) {
                topic::impl::reliable::ReliableTopicMessage message(data, std::auto_ptr<Address>());
                ringbuffer->add(message);
            }
        }
    }
}

