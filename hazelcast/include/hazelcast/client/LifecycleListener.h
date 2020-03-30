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
// Created by sancar koyunlu on 6/17/13.





#ifndef HAZELCAST_LIFECYCLE_LISTENER
#define HAZELCAST_LIFECYCLE_LISTENER

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/LifecycleEvent.h"

namespace hazelcast {
    namespace client {
        /**
         * Listener object for listening lifecycle events of hazelcast instance
         *
         *
         * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
         * otherwise it will slow down the system.
         *
         * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
         *
         * @see LifecycleEvent
         * @see HazelcastClient::addLifecycleListener(LifecycleListener *lifecycleListener)
         *
         */
        class HAZELCAST_API LifecycleListener {
        public:
            /**
             * Called when instance's state changes
             * @param lifecycleEvent LifecycleEvent
             *
             */
            virtual void stateChanged(const LifecycleEvent &lifecycleEvent) = 0;

            virtual ~LifecycleListener();
        };
    }
}


#endif //HAZELCAST_LIFECYCLE_LISTENER

