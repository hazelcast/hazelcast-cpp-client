//
// Created by sancar koyunlu on 6/20/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TOPIC
#define HAZELCAST_TOPIC

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/topic/PublishRequest.h"
#include "hazelcast/client/topic/AddMessageListenerRequest.h"
#include "hazelcast/client/topic/RemoveMessageListenerRequest.h"
#include "hazelcast/client/topic/TopicEventHandler.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/proxy/DistributedObject.h"
#include <string>

namespace hazelcast {
    namespace client {
        template <typename E>
        class HAZELCAST_API ITopic : public proxy::DistributedObject {
            friend class HazelcastClient;

        public:

            void publish(E message) {
                serialization::Data data = getContext().getSerializationService().template toData<E>(&message);
                topic::PublishRequest *request = new topic::PublishRequest(getName(), data);
                invoke<bool>(request, key);
            }

            template <typename L>
            std::string addMessageListener(L &listener) {
                topic::AddMessageListenerRequest *request = new topic::AddMessageListenerRequest(getName());
                topic::TopicEventHandler<E, L> *topicEventHandler = new topic::TopicEventHandler<E, L>(getName(), getContext().getClusterService(), getContext().getSerializationService(), listener);
                return listen(request, &key, topicEventHandler);
            }

            bool removeMessageListener(const std::string &registrationId) {
                topic::RemoveMessageListenerRequest *request = new topic::RemoveMessageListenerRequest(getName(), registrationId);
                return stopListening(request, registrationId);
            };

            void onDestroy() {
            };

        private:
            ITopic(const std::string &instanceName, spi::ClientContext *context)
            : DistributedObject("hz:impl:topicService", instanceName, context)
            , key(getContext().getSerializationService().template toData<std::string>(&instanceName)) {

            };

            serialization::Data key;
        };
    }
}

#endif //HAZELCAST_TOPIC
