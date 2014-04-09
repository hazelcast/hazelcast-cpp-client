//
// Created by sancar koyunlu on 6/20/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TOPIC
#define HAZELCAST_TOPIC

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/topic/PublishRequest.h"
#include "hazelcast/client/topic/AddMessageListenerRequest.h"
#include "hazelcast/client/topic/RemoveMessageListenerRequest.h"
#include "hazelcast/client/topic/TopicEventHandler.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/DistributedObject.h"
#include <string>

namespace hazelcast {
    namespace client {

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
        template <typename E>
        class HAZELCAST_API ITopic : public DistributedObject {
            friend class HazelcastClient;

        public:

            /**
             * Publishes the message to all subscribers of this topic
             *
             * @param message
             */
            void publish(const E &message) {
                serialization::pimpl::Data data = getContext().getSerializationService().template toData<E>(&message);
                topic::PublishRequest *request = new topic::PublishRequest(getName(), data);
                invoke<serialization::pimpl::Void>(request, partitionId);
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
             *          void onMessage(topic::Message<std::string> message) {
             *              /....
             *          }
             *     }
             *
             * @param listener
             *
             * @return returns registration id.
             */
            template <typename L>
            std::string addMessageListener(L &listener) {
                topic::AddMessageListenerRequest *request = new topic::AddMessageListenerRequest(getName());
                topic::TopicEventHandler<E, L> *topicEventHandler = new topic::TopicEventHandler<E, L>(getName(), getContext().getClusterService(), getContext().getSerializationService(), listener);
                return listen(request, partitionId, topicEventHandler);
            }

            /**
             * Stops receiving messages for the given message listener. If the given listener already removed,
             * this method does nothing.
             *
             * @param registrationId Id of listener registration.
             *
             * @return true if registration is removed, false otherwise
             */
            bool removeMessageListener(const std::string &registrationId) {
                topic::RemoveMessageListenerRequest *request = new topic::RemoveMessageListenerRequest(getName(), registrationId);
                return stopListening(request, registrationId);
            };

        private:
            ITopic(const std::string &instanceName, spi::ClientContext *context)
            : DistributedObject("hz:impl:topicService", instanceName, context) {
                serialization::pimpl::Data keyData = getContext().getSerializationService().template toData<std::string>(&instanceName);
                partitionId = getPartitionId(keyData);
            };

            void onDestroy() {
            };

            int partitionId;
        };
    }
}

#endif //HAZELCAST_TOPIC

