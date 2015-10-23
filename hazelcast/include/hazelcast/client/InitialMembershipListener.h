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
// Created by sancar koyunlu on 22/01/14.
//


#ifndef HAZELCAST_InitialMembershipListener
#define HAZELCAST_InitialMembershipListener

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        class InitialMembershipEvent;

        class MembershipEvent;

        class MemberAttributeEvent;

        /**
         * The InitializingMembershipListener is a MembershipListener that will first receives a
         * InitialMembershipEvent when it is registered so it immediately knows which members are available. After
         * that event has been received, it will receive the normal MembershipEvents.
         *
         * When the InitializingMembershipListener already is registered on a Cluster and is registered again on the same
         * Cluster instance, it will not receive an additional MembershipInitializeEvent. So this is a once only event.
         *
         * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
         * otherwise it will slow down the system.
         *
         * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
         *
         * @see Cluster#addMembershipListener(InitialMembershipListener *listener)
         * @see MembershipEvent#getMembers()
         */
        class HAZELCAST_API InitialMembershipListener {
        public:
            virtual ~InitialMembershipListener();

            /**
             * Is called when this listener is registered.
             *
             * @param event the InitialMembershipEvent
             */
            virtual void init(const InitialMembershipEvent &event) = 0;

            /**
            * Invoked when a new member is added to the cluster.
            *
            * @param membershipEvent membership event
            */
            virtual void memberAdded(const MembershipEvent &membershipEvent) = 0;

            /**
             * Invoked when an existing member leaves the cluster.
             *
             * @param membershipEvent membership event
             */
            virtual void memberRemoved(const MembershipEvent &membershipEvent) = 0;

            /**
             * Invoked when an attribute of a member was changed.
             *
             * @param memberAttributeEvent member attribute event
             */
            virtual void memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent) = 0;
        };

    }
}


#endif //HAZELCAST_InitialMembershipListener

