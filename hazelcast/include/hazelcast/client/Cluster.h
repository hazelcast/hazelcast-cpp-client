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
            namespace impl {
                class ClientClusterServiceImpl;
            }
        }

        class MembershipListener;

        /**
         * Hazelcast cluster interface.
         */
        class HAZELCAST_API Cluster {
        public:
            /**
             * Constructor
             */
            Cluster(spi::impl::ClientClusterServiceImpl &cluster_service);

            /**
             * Adds MembershipListener to listen for membership updates.
             * <p>
             * The addMembershipListener method returns a registeration ID. This ID is needed to remove the MembershipListener using the
             * Cluster::removeMembershipListener method.
             * <p>
             *
             * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
             * otherwise it will slow down the system.
             *
             * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
             * \see Cluster::removeMembershipListener

             * \param listener MembershipListener
             * \return registration id 
             */
            boost::uuids::uuid add_membership_listener(MembershipListener &&listener);

            /**
             * Removes the specified MembershipListener.
             * <p>
             * 
             * @see #addMembershipListener(const std::shared_ptr<MembershipListener> &)
             * \param registrationId the registrationId of MembershipListener to remove
             * \return true if the registration is removed, false otherwise
             * 
             */
            bool remove_membership_listener(boost::uuids::uuid registration_id);

            /**
             * Set of current members of the cluster.
             * Returning set instance is not modifiable.
             * Every member in the cluster has the same member list in the same
             * order. First member is the oldest member.
             *
             * @return current members of the cluster
             */
            std::vector<Member> get_members();

        private:
            spi::impl::ClientClusterServiceImpl &clusterService_;
        };
    }
}



