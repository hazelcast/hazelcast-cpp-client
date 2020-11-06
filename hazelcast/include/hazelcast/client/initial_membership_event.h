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
#include <unordered_set>
#include <memory>

#include "hazelcast/client/member.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class hz_cluster;

        /**
         * A event that is sent when a MembershipListener is registered.
         *
         * @see MembershipListener
         * @see MembershipEvent
         */
        class HAZELCAST_API initial_membership_event {
        public:
            initial_membership_event(hz_cluster &cluster, std::unordered_set<member> members);

            /**
             * Returns an immutable set of ordered members at the moment this MembershipListener is
             * registered. See Cluster#getMembers() for more information.
             *
             * @return a set of members.
             */
            const std::unordered_set<member> &get_members() const;

            /**
             * Returns the cluster of the event.
             *
             * @return the cluster of the event.
             */
            hz_cluster &get_cluster();

        private:
            hz_cluster &cluster_;
            std::unordered_set<member> members_;
        };

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



