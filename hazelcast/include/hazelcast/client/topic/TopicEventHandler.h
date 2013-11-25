//
// Created by sancar koyunlu on 6/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TopicEventHandler
#define HAZELCAST_TopicEventHandler

#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/topic/PortableMessage.h"
#include "hazelcast/client/topic/Message.h"
#include "hazelcast/client/serialization/SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace topic {
            template<typename E, typename L>
            class TopicEventHandler {
            public:
                TopicEventHandler(const std::string &instanceName, spi::ClusterService &clusterService, serialization::SerializationService &serializationService, L &listener)
                :instanceName(instanceName)
                , clusterService(clusterService)
                , serializationService(serializationService)
                , listener(listener) {

                };

                void handle(const PortableMessage &event) {
                    connection::Member member = clusterService.getMember(event.getUuid());
                    boost::shared_ptr<E> object = serializationService.toObject<E>(event.getMessage());
                    Message<E> message(instanceName, *object, event.getPublishTime(), member);
                    listener.onMessage(message);
                };

            private:
                const std::string &instanceName;
                serialization::SerializationService &serializationService;
                spi::ClusterService &clusterService;
                L &listener;
            };

        }
    }
}
#endif //HAZELCAST_TopicEventHandler
