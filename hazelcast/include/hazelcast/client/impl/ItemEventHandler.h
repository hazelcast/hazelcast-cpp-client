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

#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
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
                ItemEventHandler(const std::string &instanceName, spi::impl::ClientClusterServiceImpl &clusterService,
                                 serialization::pimpl::SerializationService &serializationService,
                                 ItemListener &&listener, bool includeValue)
                        : instanceName_(instanceName), clusterService_(clusterService),
                          serializationService_(serializationService), listener_(std::move(listener)), includeValue_(includeValue) {};

                void handle_item(const boost::optional<serialization::pimpl::Data> &item, boost::uuids::uuid uuid,
                                        int32_t eventType) override {
                    TypedData val;
                    if (includeValue_) {
                        val = TypedData(std::move(*item), serializationService_);
                    }
                    auto member = clusterService_.get_member(uuid);
                    ItemEventType type(static_cast<ItemEventType>(eventType));
                    ItemEvent itemEvent(instanceName_, type, std::move(val), std::move(member).value());
                    if (type == ItemEventType::ADDED) {
                        listener_.added_(std::move(itemEvent));
                    } else if (type == ItemEventType::REMOVED) {
                        listener_.removed_(std::move(itemEvent));
                    }
                }

            private:
                const std::string &instanceName_;
                spi::impl::ClientClusterServiceImpl &clusterService_;
                serialization::pimpl::SerializationService &serializationService_;
                ItemListener listener_;
                bool includeValue_;
            };
        }
    }
}

