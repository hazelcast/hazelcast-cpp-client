/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/member.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                class ClientClusterServiceImpl;
            }
        }

        class membership_listener;

        /**
         * Hazelcast cluster interface.
         */
        class HAZELCAST_API cluster {
        public:
            /**
             * Constructor
             */
            cluster(spi::impl::ClientClusterServiceImpl &cluster_service);

            /**
             * Adds membership_listener to listen for membership updates.
             * <p>
             * The add_membership_listener method returns a registeration ID. This ID is needed to remove the membership_listener using the
             * Cluster::removemembership_listener method.
             * <p>
             *
             * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
             * otherwise it will slow down the system.
             *
             * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
             * \see hz_cluster::remove_membership_listener

             * \param listener membership_listener
             * \return registration id 
             */
            boost::uuids::uuid add_membership_listener(membership_listener &&listener);

            /**
             * Removes the specified membership_listener.
             * <p>
             * 
             * @see #add_membership_listener(const std::shared_ptr<membership_listener> &)
             * \param registrationId the registrationId of membership_listener to remove
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
            std::vector<member> get_members();

        private:
            spi::impl::ClientClusterServiceImpl &cluster_service_;
        };
    }
}



