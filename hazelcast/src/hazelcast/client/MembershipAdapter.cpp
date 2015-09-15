//
// Created by sancar koyunlu on 26/08/14.
//

#include "hazelcast/client/MembershipEvent.h"
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/MembershipAdapter.h"

namespace hazelcast {
    namespace client {

        MembershipAdapter::~MembershipAdapter() {
        }

        void MembershipAdapter::memberAdded(const MembershipEvent& membershipEvent) {
        }

        void MembershipAdapter::memberRemoved(const MembershipEvent& membershipEvent) {
        }

        void MembershipAdapter::memberAttributeChanged(const MemberAttributeEvent& memberAttributeEvent) {
        }
    }
}

