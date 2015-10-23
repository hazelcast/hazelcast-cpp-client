/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 6/27/13.





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

