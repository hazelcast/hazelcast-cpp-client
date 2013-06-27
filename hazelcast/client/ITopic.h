//
// Created by sancar koyunlu on 6/20/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TOPIC
#define HAZELCAST_TOPIC

#include "spi/ClientContext.h"
#include "serialization/Data.h"
#include "serialization/SerializationService.h"
#include "topic/PublishRequest.h"
#include "topic/DestroyRequest.h"
#include "topic/AddMessageListenerRequest.h"
#include "TopicEventHandler.h"
#include <string>

namespace hazelcast {
    namespace client {
        template <typename E>
        class ITopic {
        public:

            void init(const std::string& instanceName, spi::ClientContext *clientContext) {
                this->context = clientContext;
                this->instanceName = instanceName;
                key = context->getSerializationService().toData(instanceName);
            };

            void publish(E message) {
                serialization::Data data = context->getSerializationService().toData(message);
                topic::PublishRequest request(instanceName, data);
                invoke<bool>(request);
            }

            template <typename L>
            long addMessageListener(L& listener) {
                topic::AddMessageListenerRequest request(instanceName);
                topic::TopicEventHandler<E, L> topicEventHandler(instanceName, context->getClusterService(), context->getSerializationService(), listener);
                return context->getServerListenerService().template listen<topic::AddMessageListenerRequest, topic::TopicEventHandler<E, L>, topic::PortableMessage >(instanceName, request, key, topicEventHandler);
            }

            bool removeMessageListener(long registrationId) {
                return context->getServerListenerService().stopListening(instanceName, registrationId);
            };

            void onDestroy() {
                topic::DestroyRequest request(instanceName);
                invoke<bool>(request);
            };

            std::string getName() {
                return instanceName;
            };

        private:
            template<typename Response, typename Request>
            Response invoke(const Request& request) {
                return context->getInvocationService().template invokeOnKeyOwner<Response>(request, key);
            };

            std::string instanceName;
            serialization::Data key;
            spi::ClientContext *context;
        };
    }
}

#endif //HAZELCAST_TOPIC
