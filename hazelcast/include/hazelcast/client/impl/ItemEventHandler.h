/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 6/24/13.

#ifndef HAZELCAST_ITEM_EVENT_HANDLER
#define HAZELCAST_ITEM_EVENT_HANDLER

#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/ItemEvent.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/impl/BaseEventHandler.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            template<typename E, typename BaseType>
            class ItemEventHandler : public BaseType {
            public:
                ItemEventHandler(const std::string &instanceName, spi::ClientClusterService &clusterService,
                                 serialization::pimpl::SerializationService &serializationService,
                                 ItemListener<E> &listener, bool includeValue)
                        : instanceName(instanceName), clusterService(clusterService),
                          serializationService(serializationService), listener(listener), includeValue(includeValue) {

                };

                virtual void handleItemEventV10(std::unique_ptr<serialization::pimpl::Data> &item, const std::string &uuid,
                                        const int32_t &eventType) {
                    std::shared_ptr<E> obj;
                    if (includeValue) {
                        obj = serializationService.toObject<E>(*item);
                    }
                    std::shared_ptr<Member> member = clusterService.getMember(uuid);
                    ItemEventType type((ItemEventType::Type) eventType);
                    ItemEvent<E> itemEvent(instanceName, type, *obj, *member);
                    if (type == ItemEventType::ADDED) {
                        listener.itemAdded(itemEvent);
                    } else if (type == ItemEventType::REMOVED) {
                        listener.itemRemoved(itemEvent);
                    }
                }

            private:
                const std::string &instanceName;
                spi::ClientClusterService &clusterService;
                serialization::pimpl::SerializationService &serializationService;
                ItemListener<E> &listener;
                bool includeValue;
            };
        }

        namespace mixedtype {
            namespace impl {
                template<typename BaseType>
                class MixedItemEventHandler : public BaseType {
                public:
                    MixedItemEventHandler(const std::string &instanceName, spi::ClientClusterService &clusterService,
                                          serialization::pimpl::SerializationService &serializationService,
                                          MixedItemListener &listener)
                            : instanceName(instanceName), clusterService(clusterService),
                              serializationService(serializationService), listener(listener) {
                    }

                    virtual void handleItemEventV10(std::unique_ptr<serialization::pimpl::Data> &item, const std::string &uuid,
                                            const int32_t &eventType) {
                        std::shared_ptr<Member> member = clusterService.getMember(uuid);
                        ItemEventType type((ItemEventType::Type) eventType);
                        ItemEvent<TypedData> itemEvent(instanceName, type, TypedData(item, serializationService), *member);
                        if (type == ItemEventType::ADDED) {
                            listener.itemAdded(itemEvent);
                        } else if (type == ItemEventType::REMOVED) {
                            listener.itemRemoved(itemEvent);
                        }
                    }

                private:
                    const std::string &instanceName;
                    spi::ClientClusterService &clusterService;
                    serialization::pimpl::SerializationService &serializationService;
                    MixedItemListener &listener;
                };
            }
        }
    }
}

#endif //HAZELCAST_ITEM_EVENT_HANDLER

