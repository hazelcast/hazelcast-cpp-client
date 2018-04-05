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
#include "hazelcast/client/InitialMembershipListener.h"

namespace hazelcast {
    namespace client {
        InitialMembershipListener::~InitialMembershipListener() {
        }

        bool InitialMembershipListener::shouldRequestInitialMembers() const {
            return true;
        }

        void InitialMembershipListenerDelegator::init(
                const InitialMembershipEvent &event) {
            listener->init(event);
        }

        void InitialMembershipListenerDelegator::memberRemoved(
                const MembershipEvent &membershipEvent) {
            listener->memberRemoved(membershipEvent);
        }

        void InitialMembershipListenerDelegator::memberAdded(
                const MembershipEvent &membershipEvent) {
            listener->memberAdded(membershipEvent);
        }

        void InitialMembershipListenerDelegator::memberAttributeChanged(
                const MemberAttributeEvent &memberAttributeEvent) {
            listener->memberAttributeChanged(memberAttributeEvent);
        }

        InitialMembershipListenerDelegator::InitialMembershipListenerDelegator(
                InitialMembershipListener *listener) : listener(listener) {}

    }
}
