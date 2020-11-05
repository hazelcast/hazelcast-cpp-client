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
#pragma once

#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/topic/Message.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/topic/Listener.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace topic {
            namespace impl {
                class TopicEventHandlerImpl : public protocol::codec::topic_addmessagelistener_handler {
                public:
                    TopicEventHandlerImpl(const std::string &instanceName, spi::impl::ClientClusterServiceImpl &clusterService,
                                          serialization::pimpl::SerializationService &serializationService,
                                          Listener &&messageListener)
                            :instanceName_(instanceName), clusterService_(clusterService),
                            serializationService_(serializationService), listener_(std::move(messageListener)) {}

                    void handle_topic(Data const & item, int64_t publishTime, boost::uuids::uuid uuid) override {
                        listener_.received_(Message(instanceName_, TypedData(std::move(item), serializationService_), publishTime,
                                        clusterService_.get_member(uuid)));
                    }
                private:
                    std::string instanceName_;
                    spi::impl::ClientClusterServiceImpl &clusterService_;
                    serialization::pimpl::SerializationService &serializationService_;
                    Listener listener_;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



