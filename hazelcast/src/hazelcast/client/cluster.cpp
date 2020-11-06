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

#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/MembershipEvent.h"
#include "hazelcast/client/impl/RoundRobinLB.h"
#include "hazelcast/client/cluster/impl/VectorClock.h"
#include "hazelcast/client/cluster/memberselector/MemberSelectors.h"
#include "hazelcast/client/internal/partition/strategy/StringPartitioningStrategy.h"

namespace hazelcast {
    namespace client {
        Cluster::Cluster(spi::impl::ClientClusterServiceImpl &cluster_service)
                : cluster_service_(cluster_service) {
        }

        std::vector<Member> Cluster::get_members() {
            return cluster_service_.get_member_list();
        }

        boost::uuids::uuid Cluster::add_membership_listener(MembershipListener &&listener) {
            return cluster_service_.add_membership_listener(std::move(listener));
        }

        bool Cluster::remove_membership_listener(boost::uuids::uuid registration_id) {
            return cluster_service_.remove_membership_listener(registration_id);
        }

        Member::Member() : lite_member_(false) {
        }

        Member::Member(Address address, boost::uuids::uuid uuid, bool lite, std::unordered_map<std::string, std::string> attr) :
                address_(address), uuid_(uuid), lite_member_(lite), attributes_(attr) {
        }

        Member::Member(Address member_address) : address_(member_address), lite_member_(false) {
        }

        Member::Member(boost::uuids::uuid uuid) : uuid_(uuid), lite_member_(false) {
        }

        bool Member::operator==(const Member &rhs) const {
            return uuid_ == rhs.uuid_;
        }

        const Address &Member::get_address() const {
            return address_;
        }

        boost::uuids::uuid Member::get_uuid() const {
            return uuid_;
        }

        bool Member::is_lite_member() const {
            return lite_member_;
        }

        const std::unordered_map<std::string, std::string> &Member::get_attributes() const {
            return attributes_;
        }

        std::ostream &operator<<(std::ostream &out, const Member &member) {
            const Address &address = member.get_address();
            out << "Member[";
            out << address.get_host();
            out << "]";
            out << ":";
            out << address.get_port();
            out << " - " << boost::uuids::to_string(member.get_uuid());
            return out;
        }

        const std::string *Member::get_attribute(const std::string &key) const {
            std::unordered_map<std::string, std::string>::const_iterator it = attributes_.find(key);
            if (attributes_.end() != it) {
                return &(it->second);
            } else {
                return NULL;
            }
        }

        bool Member::lookup_attribute(const std::string &key) const {
            return attributes_.find(key) != attributes_.end();
        }

        bool Member::operator<(const Member &rhs) const {
            return uuid_ < rhs.uuid_;
        }

        Endpoint::Endpoint(boost::uuids::uuid uuid, boost::optional<Address> socket_address)
                : uuid_(uuid), socket_address_(std::move(socket_address)) {}

        boost::uuids::uuid Endpoint::get_uuid() const {
            return uuid_;
        }

        const boost::optional<Address> &Endpoint::get_socket_address() const {
            return socket_address_;
        }

        MembershipEvent::MembershipEvent(Cluster &cluster, const Member &member, MembershipEventType event_type,
                                         const std::unordered_map<boost::uuids::uuid, Member, boost::hash<boost::uuids::uuid>> &members_list) :
                cluster_(cluster), member_(member), event_type_(event_type), members_(members_list) {
        }

        MembershipEvent::~MembershipEvent() = default;

        std::unordered_map<boost::uuids::uuid, Member, boost::hash<boost::uuids::uuid>> MembershipEvent::get_members() const {
            return members_;
        }

        const Cluster &MembershipEvent::get_cluster() const {
            return cluster_;
        }

        MembershipEvent::MembershipEventType MembershipEvent::get_event_type() const {
            return event_type_;
        }

        const Member &MembershipEvent::get_member() const {
            return member_;
        }

        Client::Client(boost::uuids::uuid uuid, boost::optional<Address> socket_address, std::string name,
                       std::unordered_set<std::string> labels) : Endpoint(uuid, std::move(socket_address)), name_(std::move(name)),
                                                                 labels_(std::move(labels)) {}

        const std::string &Client::get_name() const {
            return name_;
        }

        namespace impl {
            RoundRobinLB::RoundRobinLB() = default;

            void RoundRobinLB::init(Cluster &cluster) {
                AbstractLoadBalancer::init(cluster);
            }

            boost::optional<Member> RoundRobinLB::next() {
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

            void AbstractLoadBalancer::init(Cluster &cluster) {
                this->cluster_ = &cluster;
                set_members_ref();

                cluster.add_membership_listener(
                    MembershipListener()
                        .on_init([this](const InitialMembershipEvent &){
                            set_members_ref();
                        })
                        .on_joined([this](const MembershipEvent &){
                            set_members_ref();
                        })
                        .on_left([this](const MembershipEvent &){
                            set_members_ref();
                        })
                );
            }

            void AbstractLoadBalancer::set_members_ref() {
                std::lock_guard<std::mutex> lg(members_lock_);
                members_ref_ = cluster_->get_members();
            }

            std::vector<Member> AbstractLoadBalancer::get_members() {
                std::lock_guard<std::mutex> lg(members_lock_);
                return members_ref_;
            }

            AbstractLoadBalancer::~AbstractLoadBalancer() = default;

            AbstractLoadBalancer::AbstractLoadBalancer() : cluster_(NULL) {
            }
        }

        namespace cluster {
            namespace memberselector {
                bool MemberSelectors::DataMemberSelector::select(const Member &member) const {
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
    std::size_t hash<hazelcast::client::Member>::operator()(const hazelcast::client::Member &k) const noexcept {
        return boost::hash<boost::uuids::uuid>()(k.get_uuid());
    }
}

