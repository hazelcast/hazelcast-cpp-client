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
// Created by sancar koyunlu on 6/17/13.




#ifndef HAZELCAST_LIFECYCLE_EVENT
#define HAZELCAST_LIFECYCLE_EVENT

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        /**
         * Event to be fired when lifecycle states are changed.
         *
         *      enum LifeCycleState {
         *          STARTING,
         *          STARTED,
         *          SHUTTING_DOWN,
         *          SHUTDOWN,
         *          CLIENT_CONNECTED,
         *          CLIENT_DISCONNECTED
         *      };
         *
         * @see HazelcastInstance#getLifecycleService()
         * @see LifecycleService#addLifecycleListener(LifecycleListener *lifecycleListener)
         */
        class HAZELCAST_API LifecycleEvent {
        public:
            /**
             * State enum.
             */
            enum LifeCycleState {
                STARTING,
                STARTED,
                SHUTTING_DOWN,
                SHUTDOWN,
                CLIENT_CONNECTED,
                CLIENT_DISCONNECTED
            };

            /**
             * Constructor
             */
            LifecycleEvent(LifeCycleState state);

            /**
             * @return State;
             */
            LifeCycleState getState() const;

        private:
            LifeCycleState state;
        };
    }
}


#endif //__LifecycleEvent_H_

