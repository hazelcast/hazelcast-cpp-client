//
// Created by sancar koyunlu on 26/08/14.
//


#ifndef HAZELCAST_MembershipAdapter
#define HAZELCAST_MembershipAdapter

#include "hazelcast/client/MembershipListener.h"

namespace hazelcast {
    namespace client {

        class MembershipEvent;

        class MemberAttributeEvent;

        /**
        * Adapter for MembershipListener.
        * @see MembershipListener
        */
        class HAZELCAST_API MembershipAdapter : public MembershipListener {
        public:
            virtual ~MembershipAdapter() {

            };

            virtual void memberAdded(const MembershipEvent &membershipEvent);

            virtual void memberRemoved(const MembershipEvent &membershipEvent);

            virtual void memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent);

        };
    }
}
#endif //HAZELCAST_MembershipAdapter
