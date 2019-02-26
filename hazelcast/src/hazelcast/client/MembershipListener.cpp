/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#include <string>
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/MembershipListener.h"

namespace hazelcast {
    namespace client {
        MembershipListener::~MembershipListener() {
        }

        const std::string &MembershipListener::getRegistrationId() const {
            return registrationId;
        }

        void MembershipListener::setRegistrationId(const std::string &registrationId) {
            this->registrationId = registrationId;
        }

        bool MembershipListener::shouldRequestInitialMembers() const {
            return false;
        }

        MembershipListenerDelegator::MembershipListenerDelegator(
                MembershipListener *listener) : listener(listener) {}

        void MembershipListenerDelegator::memberAdded(
                const MembershipEvent &membershipEvent) {
            listener->memberAdded(membershipEvent);
        }

        void MembershipListenerDelegator::memberRemoved(
                const MembershipEvent &membershipEvent) {
            listener->memberRemoved(membershipEvent);
        }

        void MembershipListenerDelegator::memberAttributeChanged(
                const MemberAttributeEvent &memberAttributeEvent) {
            listener->memberAttributeChanged(memberAttributeEvent);
        }

        bool MembershipListenerDelegator::shouldRequestInitialMembers() const {
            return listener->shouldRequestInitialMembers();
        }

        void MembershipListenerDelegator::setRegistrationId(const std::string &registrationId) {
            listener->setRegistrationId(registrationId);
        }

        const std::string &MembershipListenerDelegator::getRegistrationId() const {
            return listener->getRegistrationId();
        }
    }
}
