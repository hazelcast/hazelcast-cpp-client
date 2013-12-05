//
// Created by sancar koyunlu on 6/20/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TOPIC
#define HAZELCAST_TOPIC

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/topic/PublishRequest.h"
#include "hazelcast/client/topic/AddMessageListenerRequest.h"
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
                topic::PublishRequest request(getName(), data);
                invoke<bool>(request);
            }

            template <typename L>
            long addMessageListener(L &listener) {
                topic::AddMessageListenerRequest request(getName());
                topic::TopicEventHandler<E, L> topicEventHandler(getName(), getContext().getClusterService(), getContext().getSerializationService(), listener);
                serialization::Data cloneData = key.clone();
                return getContext().getServerListenerService().template listen<topic::AddMessageListenerRequest, topic::TopicEventHandler<E, L>, topic::PortableMessage >(request, cloneData, topicEventHandler);
            }

            bool removeMessageListener(long registrationId) {
                return getContext().getServerListenerService().stopListening(registrationId);
            };

            void onDestroy() {
            };

        private:
            template<typename Response, typename Request>
            Response invoke(const Request &request) {
                return getContext().getInvocationService().template invokeOnKeyOwner<Response>(request, key);
            };

            ITopic(const std::string &instanceName, spi::ClientContext *context)
            : DistributedObject("hz:impl:topicService", instanceName, context)
            , key(getContext().getSerializationService().template toData<std::string>(&instanceName)) {

            };

            serialization::Data key;
        };
    }
}

#endif //HAZELCAST_TOPIC
