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
// Created by sancar koyunlu on 22/01/14.
//


#ifndef HAZELCAST_InitialMembershipEvent
#define HAZELCAST_InitialMembershipEvent

#include <vector>
#include <set>
#include <boost/shared_ptr.hpp>

#include "hazelcast/client/Member.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class Cluster;

        /**
         * A event that is send when a InitialMembershipListener} registers itself on a Cluster. For more
         * information see the InitialMembershipListener.
         *
         * @see InitialMembershipListener
         * @see MembershipListener
         * @see MembershipEvent
         */
        class HAZELCAST_API InitialMembershipEvent {
        public:
            InitialMembershipEvent(Cluster &cluster, const std::vector<Member> &members);

            InitialMembershipEvent(Cluster &cluster, const std::set<Member> &members);

            /**
             * Returns an immutable set of ordered members at the moment this InitialMembershipListener is
             * registered. See Cluster#getMembers() for more information.
             *
             * @return a set of members.
             */
            const std::vector<Member> &getMembers() const;

            /**
             * Returns the cluster of the event.
             *
             * @return the cluster of the event.
             */
            Cluster &getCluster();

        private:
            std::vector<Member> members;
            Cluster &cluster;
        };

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_InitialMembershipEvent

