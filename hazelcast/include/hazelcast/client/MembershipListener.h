//
//  MembershipListener.h
//  cpp-client
//
//  Created by sancar koyunlu on 5/30/13.
//  Copyright (c) 2013 hazelcast. All rights reserved.
//

#ifndef HAZELCAST_MEMBERSHIP_LISTENER
#define HAZELCAST_MEMBERSHIP_LISTENER

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {

        class MembershipEvent;

        class MemberAttributeEvent;

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

        };
    }
}
#endif /* HAZELCAST_MEMBERSHIP_LISTENER */

