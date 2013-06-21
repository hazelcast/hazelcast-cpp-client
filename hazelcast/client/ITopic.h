//
// Created by sancar koyunlu on 6/20/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TOPIC
#define HAZELCAST_TOPIC

#include "spi/ClientContext.h"
#include "serialization/Data.h"
#include "serialization/SerializationService.h"
#include <string>

namespace hazelcast {
    namespace client {
        template <typename E>
        class ITopic {
        public:
            ITopic(const std::string& instanceName, spi::ClientContext& clientContext)
            : name(instanceName)
            , context(clientContext)
            , key(clientContext.getSerializationService().toData(name)) {

            };

//        public void publish(E message) {
//                final Data data = getContext().getSerializationService().toData(message);
//                PublishRequest request = new PublishRequest(name, data);
//                invoke(request);
//            }
//
//        public String addMessageListener(final MessageListener<E> listener) {
//                AddMessageListenerRequest request = new AddMessageListenerRequest(name);
//                EventHandler<PortableMessage> handler = new EventHandler<PortableMessage>() {
//                    public void handle(PortableMessage event) {
//                        E messageObject = (E)getContext().getSerializationService().toObject(event.getMessage());
//                        Member member = getContext().getClusterService().getMember(event.getUuid());
//                        Message<E> message = new Message<E>(name, messageObject, event.getPublishTime(), member);
//                        listener.onMessage(message);
//                    }
//                };
//                return listen(request, getKey(), handler);
//            }

//        bool removeMessageListener(String registrationId) {
//                return stopListening(registrationId);
//            }
//
//        void onDestroy() {
//                TopicDestroyRequest request = new TopicDestroyRequest(name);
//                invoke(request);
//            }
//
//
//
//            template<typename Response, typename Request>
//            Response invoke(const Request& request) {
//                return context.getInvocationService().template invokeOnKeyOwner<Response>(request, key);
//            };

            std::string getName() {
                return name;
            };

        private:
            std::string name;
            serialization::Data key;
            spi::ClientContext& context;
        };
    }
}

#endif //HAZELCAST_TOPIC
