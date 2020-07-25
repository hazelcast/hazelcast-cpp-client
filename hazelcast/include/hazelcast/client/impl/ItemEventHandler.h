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

#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/ItemEvent.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/impl/BaseEventHandler.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            template<typename BaseType>
            class ItemEventHandler : public BaseType {
            public:
                ItemEventHandler(const std::string &instanceName, spi::ClientClusterService &clusterService,
                                 serialization::pimpl::SerializationService &serializationService,
                                 ItemListener &&listener, bool includeValue)
                        : instanceName(instanceName), clusterService(clusterService),
                          serializationService(serializationService), listener(std::move(listener)), includeValue(includeValue) {};

                void handleItemEventV10(std::unique_ptr<serialization::pimpl::Data> &item, const std::string &uuid,
                                        const int32_t &eventType) override {
                    TypedData val;
                    if (includeValue) {
                        val = TypedData(std::move(*item), serializationService);
                    }
                    auto member = clusterService.getMember(uuid);
                    ItemEventType type(static_cast<ItemEventType>(eventType));
                    ItemEvent itemEvent(instanceName, type, std::move(val), std::move(member).value());
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
                ItemListener listener;
                bool includeValue;
            };
        }
    }
}

