//
// Created by sancar koyunlu on 6/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TopicEventHandler
#define HAZELCAST_TopicEventHandler

#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/topic/PortableMessage.h"
#include "hazelcast/client/topic/Message.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/impl/BaseEventHandler.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace topic {
            template<typename E, typename L>
            class TopicEventHandler : public impl::BaseEventHandler {
            public:
                TopicEventHandler(const std::string &instanceName, spi::ClusterService &clusterService, serialization::pimpl::SerializationService &serializationService, L &listener)
                :instanceName(instanceName)
                , clusterService(clusterService)
                , serializationService(serializationService)
                , listener(listener) {

                };

                void handle(const client::serialization::pimpl::Data &data) {
                    boost::shared_ptr<PortableMessage> event = serializationService.toObject<PortableMessage>(data);
                    handle(*event);
                }

                void handle(const PortableMessage &event) {
                    Member member = clusterService.getMember(event.getUuid());
                    boost::shared_ptr<E> object = serializationService.toObject<E>(event.getMessage());
                    Message<E> message(instanceName, *object, event.getPublishTime(), member);
                    listener.onMessage(message);
                };

            private:
                const std::string &instanceName;
                spi::ClusterService &clusterService;
                serialization::pimpl::SerializationService &serializationService;
                L &listener;
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_TopicEventHandler

