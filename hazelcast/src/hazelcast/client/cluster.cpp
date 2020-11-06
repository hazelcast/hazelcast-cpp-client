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
#include <functional>
#include <boost/uuid/uuid_io.hpp>
#include <boost/functional/hash.hpp>

#include "hazelcast/client/hz_cluster.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/membership_listener.h"
#include "hazelcast/client/initial_membership_event.h"
#include "hazelcast/client/member.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/membership_event.h"
#include "hazelcast/client/impl/RoundRobinLB.h"
#include "hazelcast/client/cluster/impl/VectorClock.h"
#include "hazelcast/client/cluster/memberselector/MemberSelectors.h"
#include "hazelcast/client/internal/partition/strategy/StringPartitioningStrategy.h"

namespace hazelcast {
    namespace client {
        hz_cluster::hz_cluster(spi::impl::ClientClusterServiceImpl &cluster_service)
                : cluster_service_(cluster_service) {
        }

        std::vector<member> hz_cluster::get_members() {
            return cluster_service_.get_member_list();
        }

        boost::uuids::uuid hz_cluster::add_membership_listener(membership_listener &&listener) {
            return cluster_service_.add_membership_listener(std::move(listener));
        }

        bool hz_cluster::remove_membership_listener(boost::uuids::uuid registration_id) {
            return cluster_service_.remove_membership_listener(registration_id);
        }

        member::member() : lite_member_(false) {
        }

        member::member(address address, boost::uuids::uuid uuid, bool lite, std::unordered_map<std::string, std::string> attr) :
                address_(address), uuid_(uuid), lite_member_(lite), attributes_(attr) {
        }

        member::member(address member_address) : address_(member_address), lite_member_(false) {
        }

        member::member(boost::uuids::uuid uuid) : uuid_(uuid), lite_member_(false) {
        }

        bool member::operator==(const member &rhs) const {
            return uuid_ == rhs.uuid_;
        }

        const address &member::get_address() const {
            return address_;
        }

        boost::uuids::uuid member::get_uuid() const {
            return uuid_;
        }

        bool member::is_lite_member() const {
            return lite_member_;
        }

        const std::unordered_map<std::string, std::string> &member::get_attributes() const {
            return attributes_;
        }

        std::ostream &operator<<(std::ostream &out, const member &member) {
            const address &address = member.get_address();
            out << "Member[";
            out << address.get_host();
            out << "]";
            out << ":";
            out << address.get_port();
            out << " - " << boost::uuids::to_string(member.get_uuid());
            return out;
        }

        const std::string *member::get_attribute(const std::string &key) const {
            std::unordered_map<std::string, std::string>::const_iterator it = attributes_.find(key);
            if (attributes_.end() != it) {
                return &(it->second);
            } else {
                return NULL;
            }
        }

        bool member::lookup_attribute(const std::string &key) const {
            return attributes_.find(key) != attributes_.end();
        }

        bool member::operator<(const member &rhs) const {
            return uuid_ < rhs.uuid_;
        }

        endpoint::endpoint(boost::uuids::uuid uuid, boost::optional<address> socket_address)
                : uuid_(uuid), socket_address_(std::move(socket_address)) {}

        boost::uuids::uuid endpoint::get_uuid() const {
            return uuid_;
        }

        const boost::optional<address> &endpoint::get_socket_address() const {
            return socket_address_;
        }

        membership_event::membership_event(hz_cluster &cluster, const member &m, membership_event_type event_type,
                                           const std::unordered_map<boost::uuids::uuid, member, boost::hash<boost::uuids::uuid>> &members_list) :
                cluster_(cluster), member_(m), event_type_(event_type), members_(members_list) {
        }

        membership_event::~membership_event() = default;

        std::unordered_map<boost::uuids::uuid, member, boost::hash<boost::uuids::uuid>> membership_event::get_members() const {
            return members_;
        }

        const hz_cluster &membership_event::get_cluster() const {
            return cluster_;
        }

        membership_event::membership_event_type membership_event::get_event_type() const {
            return event_type_;
        }

        const member &membership_event::get_member() const {
            return member_;
        }

        hz_client::hz_client(boost::uuids::uuid uuid, boost::optional<address> socket_address, std::string name,
                             std::unordered_set<std::string> labels) : endpoint(uuid, std::move(socket_address)), name_(std::move(name)),
                                                                       labels_(std::move(labels)) {}

        const std::string &hz_client::get_name() const {
            return name_;
        }

        namespace impl {
            RoundRobinLB::RoundRobinLB() = default;

            void RoundRobinLB::init(hz_cluster &cluster) {
                AbstractLoadBalancer::init(cluster);
            }

            boost::optional<member> RoundRobinLB::next() {
                auto members = get_members();
                if (members.empty()) {
                    return boost::none;
                }
                return members[++index_ % members.size()];
            }

            RoundRobinLB::RoundRobinLB(const RoundRobinLB &rhs) : index_(rhs.index_.load()) {
            }

            void RoundRobinLB::operator=(const RoundRobinLB &rhs) {
                index_.store(rhs.index_.load());
            }

            AbstractLoadBalancer::AbstractLoadBalancer(const AbstractLoadBalancer &rhs) {
                *this = rhs;
            }

            void AbstractLoadBalancer::operator=(const AbstractLoadBalancer &rhs) {
                std::lock_guard<std::mutex> lg(rhs.members_lock_);
                std::lock_guard<std::mutex> lg2(members_lock_);
                members_ref_ = rhs.members_ref_;
                cluster_ = rhs.cluster_;
            }

            void AbstractLoadBalancer::init(hz_cluster &cluster) {
                this->cluster_ = &cluster;
                set_members_ref();

                cluster.add_membership_listener(
                        membership_listener()
                        .on_init([this](const initial_membership_event &){
                            set_members_ref();
                        })
                        .on_joined([this](const membership_event &){
                            set_members_ref();
                        })
                        .on_left([this](const membership_event &){
                            set_members_ref();
                        })
                );
            }

            void AbstractLoadBalancer::set_members_ref() {
                std::lock_guard<std::mutex> lg(members_lock_);
                members_ref_ = cluster_->get_members();
            }

            std::vector<member> AbstractLoadBalancer::get_members() {
                std::lock_guard<std::mutex> lg(members_lock_);
                return members_ref_;
            }

            AbstractLoadBalancer::~AbstractLoadBalancer() = default;

            AbstractLoadBalancer::AbstractLoadBalancer() : cluster_(NULL) {
            }
        }

        namespace cluster {
            namespace memberselector {
                bool MemberSelectors::DataMemberSelector::select(const member &member) const {
                    return !member.is_lite_member();
                }

                void MemberSelectors::DataMemberSelector::to_string(std::ostream &os) const {
                    os << "Default DataMemberSelector";
                }

                const std::unique_ptr<MemberSelector> MemberSelectors::DATA_MEMBER_SELECTOR(
                        new MemberSelectors::DataMemberSelector());
            }

            namespace impl {
                VectorClock::VectorClock() = default;

                VectorClock::VectorClock(const VectorClock::TimestampVector &replica_logical_timestamps)
                        : replica_timestamp_entries_(replica_logical_timestamps) {
                    for (const VectorClock::TimestampVector::value_type &replicaTimestamp : replica_logical_timestamps) {
                        replica_timestamps_[replicaTimestamp.first] = replicaTimestamp.second;
                    }
                }

                VectorClock::TimestampVector VectorClock::entry_set() {
                    return replica_timestamp_entries_;
                }

                bool VectorClock::is_after(VectorClock &other) {
                    bool anyTimestampGreater = false;
                    for (const VectorClock::TimestampMap::value_type &otherEntry : other.replica_timestamps_) {
                        const auto &replicaId = otherEntry.first;
                        int64_t otherReplicaTimestamp = otherEntry.second;
                        std::pair<bool, int64_t> localReplicaTimestamp = get_timestamp_for_replica(replicaId);

                        if (!localReplicaTimestamp.first ||
                            localReplicaTimestamp.second < otherReplicaTimestamp) {
                            return false;
                        } else if (localReplicaTimestamp.second > otherReplicaTimestamp) {
                            anyTimestampGreater = true;
                        }
                    }
                    // there is at least one local timestamp greater or local vector clock has additional timestamps
                    return anyTimestampGreater || other.replica_timestamps_.size() < replica_timestamps_.size();
                }

                std::pair<bool, int64_t> VectorClock::get_timestamp_for_replica(boost::uuids::uuid replica_id) {
                    if (replica_timestamps_.count(replica_id) == 0) {
                        return std::make_pair(false, -1);
                    }
                    return std::make_pair(true, replica_timestamps_[replica_id]);
                }
            }
        }

        namespace internal {
            namespace partition {
                namespace strategy {
                    std::string StringPartitioningStrategy::get_base_name(const std::string &name) {
                        size_t index_of = name.find('@');
                        if (index_of == std::string::npos) {
                            return name;
                        }
                        return name.substr(0, index_of);
                    }

                    std::string StringPartitioningStrategy::get_partition_key(const std::string &key) {
                        size_t firstIndexOf = key.find('@');
                        if (firstIndexOf == std::string::npos) {
                            return key;
                        } else {
                            return key.substr(firstIndexOf + 1);
                        }
                    }
                }
            }
        }
    }
}

namespace std {
    std::size_t hash<hazelcast::client::member>::operator()(const hazelcast::client::member &k) const noexcept {
        return boost::hash<boost::uuids::uuid>()(k.get_uuid());
    }
}

