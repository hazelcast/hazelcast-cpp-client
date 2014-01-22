//
// Created by sancar koyunlu on 22/01/14.
//


#ifndef HAZELCAST_InitialMembershipEvent
#define HAZELCAST_InitialMembershipEvent

#include "hazelcast/client/Member.h"

namespace hazelcast {
    namespace client {
        class Cluster;

        /**
         * A event that is send when a {@link InitialMembershipListener} registers itself on a {@link Cluster}. For more
         * information see the {@link InitialMembershipListener}.
         *
         * @see InitialMembershipListener
         * @see MembershipListener
         * @see MembershipEvent
         */
        class InitialMembershipEvent {
        public:

            InitialMembershipEvent(Cluster &cluster, const std::vector<Member> &members);

            /**
             * Returns an immutable set of ordered members at the moment this {@link InitialMembershipListener} is
             * registered. See {@link com.hazelcast.core.Cluster#getMembers()} for more information.
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


#endif //HAZELCAST_InitialMembershipEvent
