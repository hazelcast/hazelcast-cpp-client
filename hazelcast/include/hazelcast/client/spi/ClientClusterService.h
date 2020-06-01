/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#pragma once
#include <vector>
#include <memory>

#include "hazelcast/client/Client.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/MembershipListener.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace cluster {
            namespace memberselector {
                class MemberSelector;
            }
        }
        namespace spi {
            /**
             * @return The client interface representing the local client.
             */
            Client getLocalClient();

            /**
             * Cluster service for Hazelcast clients.
             *
             * Allows to retrieve Hazelcast members of the cluster, e.g. by their {@link Address} or UUID.
             */
            class HAZELCAST_API ClientClusterService {
            public:
                virtual ~ClientClusterService() {
                }

                /**
                 * Gets the member for the given address.
                 *
                 * @param address The address of the member to look up.
                 * @return The member that was found, or none if not found.
                 */
                virtual boost::optional<Member> getMember(const Address &address) = 0;

                /**
                 * Gets the member with the given UUID.
                 *
                 * @param uuid The UUID of the member.
                 * @return The member that was found, or none if not found.
                 */
                virtual boost::optional<Member> getMember(const std::string &uuid) = 0;

                /**
                 * Gets the collection of members.
                 *
                 * @return The collection of members.
                 */
                virtual std::vector<Member> getMemberList() = 0;

                /**
                 * Returns a collection of the members that satisfy the given {@link com.hazelcast.core.MemberSelector}.
                 *
                 * @param selector {@link com.hazelcast.core.MemberSelector} instance to filter members to return
                 * @return members that satisfy the given {@link com.hazelcast.core.MemberSelector}.
                 */
                virtual std::vector<Member>
                getMembers(const cluster::memberselector::MemberSelector &selector) = 0;

                /**
                 * @param listener The listener to be registered.
                 * @return The registration ID
                 */
                virtual std::string addMembershipListener(const std::shared_ptr<MembershipListener> &listener) = 0;

                /**
                 * @param registrationId The registrationId of the listener to be removed.
                 * @return true if successfully removed, false otherwise.
                 */
                virtual bool removeMembershipListener(const std::string &registrationId) = 0;

                /**
                 * Gets the current number of members.
                 *
                 * @return The current number of members.
                 */
                virtual int getSize() = 0;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


