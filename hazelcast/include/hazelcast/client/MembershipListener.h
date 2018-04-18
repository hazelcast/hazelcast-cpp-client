/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
//  MembershipListener.h
//  cpp-client
//
//  Created by sancar koyunlu on 5/30/13.
//  Copyright (c) 2013 hazelcast. All rights reserved.
//

#ifndef HAZELCAST_MEMBERSHIP_LISTENER
#define HAZELCAST_MEMBERSHIP_LISTENER

#include <string>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {

        class MembershipEvent;

        class MemberAttributeEvent;

        namespace spi {
            namespace impl {
                class ClientClusterServiceImpl;
            }
        }

        /**
         * Cluster membership listener.
         *
         * The MembershipListener will never be called concurrently and all MembershipListeners will receive the events
         * in the same order.
         *
         * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
         * otherwise it will slow down the system.
         *
         * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
         *
         * @see InitialMembershipListener
         * @see Cluster#addMembershipListener(MembershipListener*)
         */

        class HAZELCAST_API MembershipListener {
            friend class Cluster;
            friend class MembershipListenerDelegator;
            friend class InitialMembershipListenerDelegator;
            friend class spi::impl::ClientClusterServiceImpl;
        public:
            virtual ~MembershipListener();

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

        private:
            std::string registrationId;

            virtual bool shouldRequestInitialMembers() const;

            virtual const std::string &getRegistrationId() const;

            virtual void setRegistrationId(const std::string &registrationId);
        };

        class MembershipListenerDelegator : public MembershipListener {
        public:
            MembershipListenerDelegator(MembershipListener *listener);

            virtual void memberAdded(const MembershipEvent &membershipEvent);

            virtual void memberRemoved(const MembershipEvent &membershipEvent);

            virtual void memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent);

        protected:
            MembershipListener *listener;

            virtual bool shouldRequestInitialMembers() const;

            virtual void setRegistrationId(const std::string &registrationId);

            virtual const std::string &getRegistrationId() const;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_MEMBERSHIP_LISTENER */

