/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 04/09/14.
//


#ifndef HAZELCAST_ItemListener
#define HAZELCAST_ItemListener

#include "hazelcast/client/ItemEvent.h"

namespace hazelcast {
    namespace client {

        /**
        * Item listener for  IQueue, ISet and IList
        *
        * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
        * otherwise it will slow down the system.
        *
        * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
        *
        * @param <E> item
        */
        template<typename E>
        class ItemListener {
        public:
            virtual ~ItemListener() {
            }

            /**
            * Invoked when an item is added.
            *
            * @param item added item
            */
            virtual void itemAdded(const ItemEvent<E>& item) = 0;

            /**
            * Invoked when an item is removed.
            *
            * @param item removed item.
            */
            virtual void itemRemoved(const ItemEvent<E>& item) = 0;
        };

        class HAZELCAST_API MixedItemListener : public ItemListener<TypedData> {};
    }
}


#endif //HAZELCAST_ItemListener
