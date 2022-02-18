/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include "hazelcast/client/proxy/ITopicImpl.h"
#include "hazelcast/client/topic/impl/TopicEventHandlerImpl.h"

namespace hazelcast {
namespace client {
namespace topic {
class listener;
}

/**
 * Hazelcast provides distribution mechanism for publishing messages that are
 * delivered to multiple subscribers which is also known as publish/subscribe
 * (pub/sub) messaging model. Publish and subscriptions are cluster-wide. When a
 * member subscribes for a topic, it is actually registering for messages
 * published by any member in the cluster, including the new members joined
 * after you added the listener.
 *
 * Messages are ordered, meaning, listeners(subscribers)
 * will process the messages in the order they are actually published. If
 * cluster member M publishes messages m1, m2, m3...mn to a topic T, then
 * Hazelcast makes sure that all of the subscribers of topic T will receive and
 * process m1, m2, m3...mn in order.
 *
 */
class itopic : public proxy::ITopicImpl
{
    friend class spi::ProxyManager;

public:
    static constexpr const char* SERVICE_NAME = "hz:impl:topicService";

    /**
     * Publishes the message to all subscribers of this topic
     *
     * @param message
     */
    template<typename E>
    boost::future<void> publish(const E& message)
    {
        return proxy::ITopicImpl::publish(to_data<E>(message));
    }

    /**
     * Subscribe to this topic.
     * When a message is published on this topic,
     * the callback function that was provided to the given Listener object is
     * called. \see Listener
     *
     * Warning 1: If listener should do a time consuming operation, off-load the
     * operation to another thread. otherwise it will slow down the system.
     *
     * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
     *
     * \param listener
     *
     * \return registration id.
     */
    boost::future<boost::uuids::uuid> add_message_listener(
      topic::listener&& listener)
    {
        return proxy::ITopicImpl::add_message_listener(
          std::shared_ptr<impl::BaseEventHandler>(
            new topic::impl::TopicEventHandlerImpl(
              get_name(),
              get_context().get_client_cluster_service(),
              get_context().get_serialization_service(),
              std::move(listener))));
    }

private:
    itopic(const std::string& instance_name, spi::ClientContext* context)
      : proxy::ITopicImpl(instance_name, context)
    {}
};
} // namespace client
} // namespace hazelcast
