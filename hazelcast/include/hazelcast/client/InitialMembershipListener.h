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

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/MembershipListener.h"

namespace hazelcast {
    namespace client {
        class InitialMembershipEvent;

        class MembershipEvent;

        class MemberAttributeEvent;

        namespace spi {
            namespace impl {
                class ClientClusterServiceImpl;
            }
        }

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
        class HAZELCAST_API InitialMembershipListener : public MembershipListener {
            friend class spi::impl::ClientClusterServiceImpl;
            friend class InitialMembershipListenerDelegator;
        public:
            ~InitialMembershipListener() override;

            /**
             * Is called when this listener is registered.
             *
             * @param event the InitialMembershipEvent
             */
            virtual void init(InitialMembershipEvent event) = 0;

        protected:
            bool shouldRequestInitialMembers() const override;
        };

        class InitialMembershipListenerDelegator : public InitialMembershipListener {
        public:
            InitialMembershipListenerDelegator(InitialMembershipListener *listener);

            void init(InitialMembershipEvent event) override;

            void memberRemoved(const MembershipEvent &membershipEvent) override;

            void memberAdded(const MembershipEvent &membershipEvent) override;

        private:
            bool shouldRequestInitialMembers() const override;

            boost::uuids::uuid  getRegistrationId() const override;

            void setRegistrationId(boost::uuids::uuid registrationId) override;

            InitialMembershipListener *listener_;
        };
    }
}




