//
// Created by sancar koyunlu on 6/20/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TOPIC
#define HAZELCAST_TOPIC

#include "spi/ClientContext.h"
#include "serialization/Data.h"
#include "topic/PublishRequest.h"
#include "topic/DestroyRequest.h"
#include "topic/AddMessageListenerRequest.h"
#include "TopicEventHandler.h"
#include "serialization/SerializationService.h"
#include "hazelcast/client/spi/DistributedObjectListenerService.h"
#include <string>

namespace hazelcast {
    namespace client {
        template <typename E>
        class ITopic {
            friend class HazelcastClient;

        public:

            void publish(E message) {
                serialization::Data data = context->getSerializationService().toData<E>(&message);
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

            void destroy() {
                topic::DestroyRequest request(instanceName);
                invoke<bool>(request);
                context->getDistributedObjectListenerService().removeDistributedObject(instanceName);
            };

            std::string getName() {
                return instanceName;
            };

        private:
            template<typename Response, typename Request>
            Response invoke(const Request& request) {
                return context->getInvocationService().template invokeOnKeyOwner<Response>(request, key);
            };

            ITopic(){

            };

            void init(const std::string& instanceName, spi::ClientContext *clientContext) {
                this->context = clientContext;
                this->instanceName = instanceName;
                key = context->getSerializationService().toData<std::string>(&instanceName);
            };


            std::string instanceName;
            serialization::Data key;
            spi::ClientContext *context;
        };
    }
}

#endif //HAZELCAST_TOPIC
