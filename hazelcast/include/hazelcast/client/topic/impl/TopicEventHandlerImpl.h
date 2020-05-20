/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#pragma once
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/topic/impl/MessageImpl.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/topic/MessageListener.h"

#include <assert.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace topic {
            namespace impl {
                template<typename E>
                class TopicEventHandlerImpl : public protocol::codec::TopicAddMessageListenerCodec::AbstractEventHandler {
                public:
                    TopicEventHandlerImpl(const std::string &instanceName, spi::ClientClusterService &clusterService,
                                      serialization::pimpl::SerializationService &serializationService,
                                      MessageListener<E> &messageListener)
                            :instanceName(instanceName)
                            , clusterService(clusterService)
                            , serializationService(serializationService)
                            , listener(messageListener) {
                    }

                    virtual void handleTopicEventV10(const serialization::pimpl::Data &item, const int64_t &publishTime,
                                             const std::string &uuid) {
                        std::shared_ptr<Member> member = clusterService.getMember(uuid);

                        std::unique_ptr<E> object = serializationService.toObject<E>(item);

                        std::unique_ptr<Message<E> > listenerMsg(new impl::MessageImpl<E>(instanceName, object, publishTime, member));
                        listener.onMessage(std::move(listenerMsg));
                    }
                private:
                    const std::string &instanceName;
                    spi::ClientClusterService &clusterService;
                    serialization::pimpl::SerializationService &serializationService;
                    MessageListener<E> &listener;
                };
            }
        }
        namespace mixedtype {
            namespace topic {
                namespace impl {
                    class HAZELCAST_API TopicEventHandlerImpl : public protocol::codec::TopicAddMessageListenerCodec::AbstractEventHandler {
                    public:
                        TopicEventHandlerImpl(const std::string &instanceName, spi::ClientClusterService &clusterService,
                                              serialization::pimpl::SerializationService &serializationService,
                                              MessageListener &messageListener)
                                :instanceName(instanceName)
                                , clusterService(clusterService)
                                , serializationService(serializationService)
                                , listener(messageListener) {
                        }

                        virtual void handleTopicEventV10(const serialization::pimpl::Data &item, const int64_t &publishTime,
                                                 const std::string &uuid) {
                            std::shared_ptr<Member> member = clusterService.getMember(uuid);

                            std::unique_ptr<TypedData> object(new TypedData(
                                    std::unique_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(item)),
                                    serializationService));

                            std::unique_ptr<client::topic::Message<TypedData> > listenerMsg(
                                    new impl::MessageImpl(instanceName, object, publishTime, member));
                            listener.onMessage(std::move(listenerMsg));
                        }
                    private:
                        const std::string &instanceName;
                        spi::ClientClusterService &clusterService;
                        serialization::pimpl::SerializationService &serializationService;
                        MessageListener &listener;
                    };

                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


