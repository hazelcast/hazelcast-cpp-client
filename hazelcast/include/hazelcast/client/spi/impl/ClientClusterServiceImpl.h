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

#include <unordered_map>
#include <vector>
#include <boost/thread/latch.hpp>
#include <boost/functional/hash.hpp>
#include <boost/smart_ptr/atomic_shared_ptr.hpp>

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/Client.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            class Connection;
        }
        namespace cluster {
            namespace memberselector {
                class MemberSelector;
            }
        }
        class InitialMembershipListener;
        class InitialMembershipEvent;

        namespace spi {
            class ClientContext;

            namespace impl {
                class ClientMembershipListener;

                class HAZELCAST_API ClientClusterServiceImpl {
                public:
                    explicit ClientClusterServiceImpl(ClientContext &client);

                    void start();

                    void shutdown();

                    boost::optional<Member> getMember(boost::uuids::uuid uuid) const;

                    std::vector<Member> getMemberList() const;

                    std::vector<Member> getMembers(
                            const cluster::memberselector::MemberSelector &selector) const;

                    Client getLocalClient() const;

                    boost::uuids::uuid addMembershipListener(const std::shared_ptr<MembershipListener> &listener);

                    bool removeMembershipListener(boost::uuids::uuid registrationId);

                    void clear_member_list_version();

                    void handle_event(int32_t version, const std::vector<Member> &memberInfos);

                    void wait_initial_member_list_fetched() const;

                private:
                    static constexpr boost::chrono::steady_clock::duration INITIAL_MEMBERS_TIMEOUT{boost::chrono::seconds(120)};
                    struct member_list_snapshot {
                        int32_t version;
                        std::unordered_map<boost::uuids::uuid, Member, boost::hash<boost::uuids::uuid>> members;
                    };

                    ClientContext &client;
                    std::shared_ptr<ClientMembershipListener> clientMembershipListener;
                    util::Sync<std::unordered_map<Address, std::shared_ptr<Member> > > members_;
                    util::SynchronizedMap<boost::uuids::uuid, MembershipListener, boost::hash<boost::uuids::uuid>> listeners;
                    std::mutex cluster_view_lock_;
                    boost::atomic_shared_ptr<member_list_snapshot> member_list_snapshot_;
                    const std::unordered_set<std::string> labels_;
                    boost::latch initial_list_fetched_latch_;

                    static const boost::shared_ptr<member_list_snapshot> EMPTY_SNAPSHOT;

                    boost::uuids::uuid addMembershipListenerWithoutInit(const std::shared_ptr<MembershipListener> &listener);

                    void fireInitialMembershipEvent(const InitialMembershipEvent &event);

                    static member_list_snapshot create_snapshot(int32_t version, const std::vector<Member> &vector);

                    static std::string members_string(const member_list_snapshot& snapshot);

                    void apply_initial_state(int32_t version, const std::vector<Member> &memberInfos);

                    std::vector<MembershipEvent>
                    detect_membership_events(std::unordered_map<boost::uuids::uuid, Member, boost::hash<boost::uuids::uuid>> previous_members,
                                             const std::unordered_map<boost::uuids::uuid, Member, boost::hash<boost::uuids::uuid>>& current_members);

                    void fire_events(std::vector<MembershipEvent> events);
                };

            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
