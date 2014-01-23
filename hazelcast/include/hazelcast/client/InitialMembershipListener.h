//
// Created by sancar koyunlu on 22/01/14.
//


#ifndef HAZELCAST_InitialMembershipListener
#define HAZELCAST_InitialMembershipListener

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        class InitialMembershipEvent;

        class MembershipEvent;

        /**
         * The InitializingMembershipListener is a {@link MembershipListener} that will first receives a
         * {@link InitialMembershipEvent} when it is registered so it immediately knows which members are available. After
         * that event has been received, it will receive the normal MembershipEvents.
         *
         * When the InitializingMembershipListener already is registered on a {@link Cluster} and is registered again on the same
         * Cluster instance, it will not receive an additional MembershipInitializeEvent. So this is a once only event.
         *
         * @see Cluster#addMembershipListener(InitialMembershipListener *listener)
         * @see MembershipEvent#getMembers()
         */
        class HAZELCAST_API InitialMembershipListener {
        public:
            virtual ~InitialMembershipListener() {

            };

            /**
             * Is called when this listener is registered.
             *
             * @param event the InitialMembershipEvent
             */
            virtual void init(const InitialMembershipEvent &event) = 0;

            /**
            * Invoked when a new member is added to the cluster.
            *
            * @param membershipEvent membership event
            */
            virtual void memberAdded(const MembershipEvent &event) = 0;

            /**
             * Invoked when an existing member leaves the cluster.
             *
             * @param membershipEvent membership event
             */
            virtual void memberRemoved(const MembershipEvent &event) = 0;

            /**
             * Invoked when an attribute of a member was changed.
             *
             * @param memberAttributeEvent member attribute event
             */
//            void memberAttributeChanged(const MemberAttributeEvent& memberAttributeEvent) = 0; MTODO
        };

    }
}


#endif //HAZELCAST_InitialMembershipListener
