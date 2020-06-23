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

#include <vector>
#include <memory>

#include "hazelcast/client/Member.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientClusterService;
        }

        class MembershipListener;

        class InitialMembershipListener;

        /**
         * Hazelcast cluster interface.
         */
        class HAZELCAST_API Cluster {
        public:
            /**
             * Constructor
             */
            Cluster(spi::ClientClusterService &clusterService);

            /**
             * @deprecated Please use {@link addMembershipListener(const std::shared_ptr<MembershipListener> &)}
             *
             * Adds MembershipListener to listen for membership updates.
             *
             * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
             * otherwise it will slow down the system.
             *
             * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
             *
             * @param listener MembershipListener
             */

            void addMembershipListener(MembershipListener *listener);

            /**
             * Adds MembershipListener to listen for membership updates.
             * <p>
             * The addMembershipListener method returns a register ID. This ID is needed to remove the MembershipListener using the
             * {@link #removeMembershipListener(String)} method.
             * <p>
             * If the MembershipListener implements the {@link InitialMembershipListener} interface, it will also receive
             * the {@link InitialMembershipEvent}.
             * <p>
             * There is no check for duplicate registrations, so if you register the listener twice, it will get events twice.
             *
             * @param listener membership listener
             * @return the registration ID
             * @throws NullPointerException if listener is null
             * @see #removeMembershipListener(const std::string &)
             */
            std::string addMembershipListener(const std::shared_ptr<MembershipListener> &listener);

            /**
             * @deprecated Please use {@link addMembershipListener(const std::shared_ptr<InitialMembershipListener> &)}
             *
             * Adds InitialMembershipListener to listen for membership updates.
             *
             * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
             * otherwise it will slow down the system.
             *
             * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
             *
             * @param listener InitialMembershipListener
             * @return the registration ID
             */

            std::string addMembershipListener(InitialMembershipListener *listener);

            /**
             * Adds MembershipListener to listen for membership updates.
             * <p>
             * The addMembershipListener method returns a register ID. This ID is needed to remove the MembershipListener using the
             * {@link #removeMembershipListener(String)} method.
             * <p>
             * If the MembershipListener implements the {@link InitialMembershipListener} interface, it will also receive
             * the {@link InitialMembershipEvent}.
             * <p>
             * There is no check for duplicate registrations, so if you register the listener twice, it will get events twice.
             *
             * @param listener membership listener
             * @return the registration ID
             * @throws NullPointerException if listener is null
             * @see #removeMembershipListener(const std::string &)
             */
            std::string addMembershipListener(const std::shared_ptr<InitialMembershipListener> &listener);

            /**
             * @deprecated Please use {@link removeMembershipListener(const std::string &)}
             *
             * Removes the specified membership listener.
             *
             * @param listener MembershipListener * to be removed
             *
             * @return true if registration is removed, false otherwise
             */
            bool removeMembershipListener(MembershipListener *listener);

            /**
             * Removes the specified MembershipListener.
             * <p>
             * If the same MembershipListener is registered multiple times, it needs to be removed multiple times.
             *
             * This method can safely be called multiple times for the same registration ID; subsequent calls are ignored.
             *
             * @param registrationId the registrationId of MembershipListener to remove
             * @return true if the registration is removed, false otherwise
             * @see #addMembershipListener(const std::shared_ptr<MembershipListener> &)
             */
            bool removeMembershipListener(const std::string &registrationId);

            /**
             * Set of current members of the cluster.
             * Returning set instance is not modifiable.
             * Every member in the cluster has the same member list in the same
             * order. First member is the oldest member.
             *
             * @return current members of the cluster
             */
            std::vector<Member> getMembers();

        private:
            spi::ClientClusterService &clusterService;
        };
    }
}



