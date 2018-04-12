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
#ifndef HAZELCAST_CLIENT_MIXEDTYPE_ITOPIC_H_
#define HAZELCAST_CLIENT_MIXEDTYPE_ITOPIC_H_

#include <vector>
#include <string>

#include "hazelcast/client/proxy/ITopicImpl.h"
#include "hazelcast/client/TypedData.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            namespace topic {
                class MessageListener;
            }

            /**
            * Hazelcast provides distribution mechanism for publishing messages that are delivered to multiple subscribers
            * which is also known as publish/subscribe (pub/sub) messaging model. Publish and subscriptions are cluster-wide.
            * When a member subscribes for a topic, it is actually registering for messages published by any member in the cluster,
            * including the new members joined after you added the listener.
            *
            * Messages are ordered, meaning, listeners(subscribers)
            * will process the messages in the order they are actually published. If cluster member M publishes messages
            * m1, m2, m3...mn to a topic T, then Hazelcast makes sure that all of the subscribers of topic T will receive
            * and process m1, m2, m3...mn in order.
            *
            */
            class HAZELCAST_API ITopic : public proxy::ITopicImpl {
                friend class client::impl::HazelcastClientInstanceImpl;;

            public:

                /**
                * Publishes the message to all subscribers of this topic
                *
                * @param message
                */
                template <typename E>
                void publish(const E& message) {
                    proxy::ITopicImpl::publish(toData<E>(message));
                }

                /**
                * Subscribes to this topic. When someone publishes a message on this topic.
                * onMessage() function of the given MessageListener is called. More than one message listener can be
                * added on one instance.
                * listener class should implement onMessage function like in the following example:
                *
                *      class MyMessageListener {
                *      public:
                *          //....
                *
                *          void onMessage(topic::Message<TypedData> message) {
                *              /....
                *          }
                *     }
                *
                *  Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
                * otherwise it will slow down the system.
                *
                * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
                *
                * @param listener
                *
                * @return returns registration id.
                */
                std::string addMessageListener(topic::MessageListener &listener);

                /**
                * Stops receiving messages for the given message listener. If the given listener already removed,
                * this method does nothing.
                *
                * @param registrationId Id of listener registration.
                *
                * @return true if registration is removed, false otherwise
                */
                bool removeMessageListener(const std::string& registrationId);

            private:
                ITopic(const std::string& instanceName, spi::ClientContext *context);
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_MIXEDTYPE_ITOPIC_H_ */

