/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
            virtual ~MembershipAdapter();

            virtual void memberAdded(const MembershipEvent &membershipEvent);

            virtual void memberRemoved(const MembershipEvent &membershipEvent);

            virtual void memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent);

        };
    }
}
#endif //HAZELCAST_MembershipAdapter
