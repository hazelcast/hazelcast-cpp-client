//
// Created by sancar koyunlu on 23/01/14.
//

#ifndef HAZELCAST_MembershipEvent
#define HAZELCAST_MembershipEvent

#include "hazelcast/client/Member.h"

namespace hazelcast {
    namespace client {

        class Cluster;

        /**
         * Membership event fired when a new member is added
         * to the cluster and/or when a member leaves the cluster.
         *
         * @see MembershipListener
         */
        class HAZELCAST_API MembershipEvent {
        public:
            /**
             * MembershipEventType
             *
             * MEMBER_ADDED = 1,
             * MEMBER_REMOVED = 2,
             * MEMBER_ATTRIBUTE_CHANGED = 5
             */
            enum MembershipEventType {
                MEMBER_ADDED = 1,
                MEMBER_REMOVED = 2,
                MEMBER_ATTRIBUTE_CHANGED = 5
            };

            /**
             * Internal API.
             * Constructor.
             */
            MembershipEvent(Cluster &cluster, MembershipEventType eventType, const Member &member);
            
            /**
             * Destructor
             */
            virtual ~MembershipEvent();

            /**
             * Returns a consistent view of the the members exactly after this MembershipEvent has been processed. So if a
             * member is removed, the returned set will not include this member. And if a member is added it will include
             * this member.
             *
             * The problem with calling the Cluster#getMembers() is that the content could already
             * have changed while processing this event so it becomes very difficult to write a deterministic algorithm since
             * you can't get a deterministic view of the members. This method solves that problem.
             *
             * The set is immutable and ordered. For more information see Cluster#getMembers().
             *
             * @return the members at the moment after this event.
             */
            virtual std::vector <Member> getMembers() const;

            /**
             * Returns the cluster of the event.
             *
             * @return
             */
            virtual const Cluster &getCluster() const;

            /**
             * Returns the membership event type; MembershipEvent#MEMBER_ADDED ,
             * MembershipEvent#MEMBER_REMOVED and MembershipEvent#MEMBER_ATTRIBUTE_CHANGED
             *
             * @return the membership event type
             */
            virtual MembershipEventType getEventType() const;

            /**
             * Returns the removed or added member.
             *
             * @return member which is removed/added
             */
            virtual const Member &getMember() const;

        private:
            Cluster *cluster;
            Member member;
            MembershipEventType eventType;
        };
    }
}
#endif //HAZELCAST_MembershipEvent

