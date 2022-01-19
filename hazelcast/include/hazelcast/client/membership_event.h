/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/member.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {

class cluster;

/**
 * Membership event fired when a new member is added
 * to the cluster and/or when a member leaves the cluster.
 *
 * @see membership_listener
 */
class HAZELCAST_API membership_event
{
public:
    enum membership_event_type
    {
        MEMBER_JOINED = 1,
        MEMBER_LEFT = 2,
    };

    /**
     * Internal API.
     * Constructor.
     */
    membership_event(
      cluster& cluster,
      const member& m,
      membership_event_type event_type,
      const std::unordered_map<boost::uuids::uuid,
                               member,
                               boost::hash<boost::uuids::uuid>>& members_list);

    /**
     * Destructor
     */
    virtual ~membership_event();

    /**
     * Returns a consistent view of the the members exactly after this
     * MembershipEvent has been processed. So if a member is removed, the
     * returned vector will not include this member. And if a member is added it
     * will include this member.
     *
     * The problem with calling the Cluster#getMembers() is that the content
     * could already have changed while processing this event so it becomes very
     * difficult to write a deterministic algorithm since you can't get a
     * deterministic view of the members. This method solves that problem.
     *
     * The vector is immutable and ordered. For more information see
     * Cluster#getMembers().
     *
     * @return the members at the moment after this event.
     */
    std::
      unordered_map<boost::uuids::uuid, member, boost::hash<boost::uuids::uuid>>
      get_members() const;

    /**
     * Returns the cluster of the event.
     *
     * @return the cluster reference
     */
    cluster& get_cluster();

    /**
     * Returns the membership event type; MembershipEvent#MEMBER_JOINED ,
     * MembershipEvent#MEMBER_LEFT
     *
     * @return the membership event type
     */
    membership_event_type get_event_type() const;

    /**
     * Returns the removed or added member.
     *
     * @return member which is removed/added
     */
    const member& get_member() const;

private:
    cluster& cluster_;
    member member_;
    membership_event_type event_type_;
    std::
      unordered_map<boost::uuids::uuid, member, boost::hash<boost::uuids::uuid>>
        members_;
};
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
