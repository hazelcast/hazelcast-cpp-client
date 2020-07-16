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

#include <utility>
#include <functional>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        class ItemEvent;
        class ItemListener;

        /**
        * Item listener for  IQueue, ISet and IList
        *
        * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
        * otherwise it will slow down the system.
        *
        * Warning 2: Do not make a call to hazelcast. It can cause a deadlock.
        *
        */
        class HAZELCAST_API ItemListener final {
        public:
            template <typename ItemAddedHandlerT, typename ItemRemovedHandlerT>
            explicit ItemListener(ItemAddedHandlerT &&itemAdded, 
                                  ItemRemovedHandlerT &&itemRemoved)
                : itemAdded_(std::forward<ItemAddedHandlerT>(itemAdded)),
                  itemRemoved_(std::forward<ItemRemovedHandlerT>(itemRemoved)) {}

            void itemAdded(const ItemEvent &event) { 
                itemAdded_(event); 
            }

            void itemRemoved(const ItemEvent &event) { 
                itemRemoved_(event); 
            }
        private:
            using HandlerType = std::function<void(const ItemEvent &)>;
            HandlerType itemAdded_, itemRemoved_;
        };
    }
}
