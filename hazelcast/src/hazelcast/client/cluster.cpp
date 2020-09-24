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
#include "hazelcast/client/InitialMembershipListener.h"
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
        Cluster::Cluster(spi::impl::ClientClusterServiceImpl &clusterService)
                : cluster_service_(clusterService) {
        }

        void Cluster::addMembershipListener(MembershipListener *listener) {
            cluster_service_.addMembershipListener(
                    std::shared_ptr<MembershipListener>(new MembershipListenerDelegator(listener)));
        }

        bool Cluster::removeMembershipListener(MembershipListener *listener) {
            return cluster_service_.removeMembershipListener(listener->getRegistrationId());
        }

        std::vector<Member> Cluster::getMembers() {
            return cluster_service_.getMemberList();
        }

        boost::uuids::uuid Cluster::addMembershipListener(const std::shared_ptr<MembershipListener> &listener) {
            return cluster_service_.addMembershipListener(listener);
        }

        bool Cluster::removeMembershipListener(boost::uuids::uuid registrationId) {
            return cluster_service_.removeMembershipListener(registrationId);
        }

        boost::uuids::uuid Cluster::addMembershipListener(const std::shared_ptr<InitialMembershipListener> &listener) {
            return cluster_service_.addMembershipListener(listener);
        }

        boost::uuids::uuid Cluster::addMembershipListener(InitialMembershipListener *listener) {
            return cluster_service_.addMembershipListener(
                    std::shared_ptr<MembershipListener>(new InitialMembershipListenerDelegator(listener)));

        }

        Member::Member() : lite_member_(false) {
        }

        Member::Member(Address address, boost::uuids::uuid uuid, bool lite, std::unordered_map<std::string, std::string> attr) :
                address_(address), uuid_(uuid), lite_member_(lite), attributes_(attr) {
        }

        Member::Member(Address memberAddress) : address_(memberAddress), lite_member_(false) {
        }

        Member::Member(boost::uuids::uuid uuid) : uuid_(uuid), lite_member_(false) {
        }

        bool Member::operator==(const Member &rhs) const {
            return uuid_ == rhs.uuid_;
        }

        const Address &Member::getAddress() const {
            return address_;
        }

        boost::uuids::uuid Member::getUuid() const {
            return uuid_;
        }

        bool Member::isLiteMember() const {
            return lite_member_;
        }

        const std::unordered_map<std::string, std::string> &Member::getAttributes() const {
            return attributes_;
        }

        std::ostream &operator<<(std::ostream &out, const Member &member) {
            const Address &address = member.getAddress();
            out << "Member[";
            out << address.getHost();
            out << "]";
            out << ":";
            out << address.getPort();
            out << " - " << boost::uuids::to_string(member.getUuid());
            return out;
        }

        const std::string *Member::getAttribute(const std::string &key) const {
            std::unordered_map<std::string, std::string>::const_iterator it = attributes_.find(key);
            if (attributes_.end() != it) {
                return &(it->second);
            } else {
                return NULL;
            }
        }

        bool Member::lookupAttribute(const std::string &key) const {
            return attributes_.find(key) != attributes_.end();
        }

        bool Member::operator<(const Member &rhs) const {
            return uuid_ < rhs.uuid_;
        }

        Endpoint::Endpoint(boost::uuids::uuid uuid, boost::optional<Address> socketAddress)
                : uuid_(uuid), socket_address_(std::move(socketAddress)) {}

        boost::uuids::uuid Endpoint::getUuid() const {
            return uuid_;
        }

        const boost::optional<Address> &Endpoint::getSocketAddress() const {
            return socket_address_;
        }

        MembershipEvent::MembershipEvent(Cluster &cluster, const Member &member, MembershipEventType eventType,
                                         const std::unordered_map<boost::uuids::uuid, Member, boost::hash<boost::uuids::uuid>> &membersList) :
                cluster_(cluster), member_(member), event_type_(eventType), members_(membersList) {
        }

        MembershipEvent::~MembershipEvent() = default;

        std::unordered_map<boost::uuids::uuid, Member, boost::hash<boost::uuids::uuid>> MembershipEvent::getMembers() const {
            return members_;
        }

        const Cluster &MembershipEvent::getCluster() const {
            return cluster_;
        }

        MembershipEvent::MembershipEventType MembershipEvent::getEventType() const {
            return event_type_;
        }

        const Member &MembershipEvent::getMember() const {
            return member_;
        }

        Client::Client(boost::uuids::uuid uuid, boost::optional<Address> socketAddress, std::string name,
                       std::unordered_set<std::string> labels) : Endpoint(uuid, std::move(socketAddress)), name_(std::move(name)),
                                                                 labels_(std::move(labels)) {}

        const std::string &Client::getName() const {
            return name_;
        }

        MembershipListener::~MembershipListener() = default;

        boost::uuids::uuid  MembershipListener::getRegistrationId() const {
            return registrationId;
        }

        void MembershipListener::setRegistrationId(boost::uuids::uuid registrationId) {
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

        bool MembershipListenerDelegator::shouldRequestInitialMembers() const {
            return listener->shouldRequestInitialMembers();
        }

        void MembershipListenerDelegator::setRegistrationId(boost::uuids::uuid registrationId) {
            listener->setRegistrationId(registrationId);
        }

        boost::uuids::uuid  MembershipListenerDelegator::getRegistrationId() const {
            return listener->getRegistrationId();
        }

        InitialMembershipListener::~InitialMembershipListener() = default;

        bool InitialMembershipListener::shouldRequestInitialMembers() const {
            return true;
        }

        void InitialMembershipListenerDelegator::init(InitialMembershipEvent event) {
            listener_->init(std::move(event));
        }

        void InitialMembershipListenerDelegator::memberRemoved(
                const MembershipEvent &membershipEvent) {
            listener_->memberRemoved(membershipEvent);
        }

        void InitialMembershipListenerDelegator::memberAdded(
                const MembershipEvent &membershipEvent) {
            listener_->memberAdded(membershipEvent);
        }

        InitialMembershipListenerDelegator::InitialMembershipListenerDelegator(
                InitialMembershipListener *listener) : listener_(listener) {}

        bool InitialMembershipListenerDelegator::shouldRequestInitialMembers() const {
            return listener_->shouldRequestInitialMembers();
        }

        boost::uuids::uuid  InitialMembershipListenerDelegator::getRegistrationId() const {
            return listener_->getRegistrationId();
        }

        void InitialMembershipListenerDelegator::setRegistrationId(boost::uuids::uuid registrationId) {
            listener_->setRegistrationId(registrationId);
        }

        namespace impl {
            RoundRobinLB::RoundRobinLB() = default;

            void RoundRobinLB::init(Cluster &cluster) {
                AbstractLoadBalancer::init(cluster);
            }

            boost::optional<Member> RoundRobinLB::next() {
                auto members = getMembers();
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
                setMembersRef();
                cluster.addMembershipListener(this);
            }

            void AbstractLoadBalancer::setMembersRef() {
                std::lock_guard<std::mutex> lg(members_lock_);
                members_ref_ = cluster_->getMembers();
            }

            void AbstractLoadBalancer::memberAdded(const MembershipEvent &membershipEvent) {
                setMembersRef();
            }

            void AbstractLoadBalancer::memberRemoved(const MembershipEvent &membershipEvent) {
                setMembersRef();
            }

            std::vector<Member> AbstractLoadBalancer::getMembers() {
                std::lock_guard<std::mutex> lg(members_lock_);
                return members_ref_;
            }

            AbstractLoadBalancer::~AbstractLoadBalancer() = default;

            AbstractLoadBalancer::AbstractLoadBalancer() : cluster_(NULL) {
            }

            void AbstractLoadBalancer::init(InitialMembershipEvent event) {
                setMembersRef();
            }
        }

        namespace cluster {
            namespace memberselector {
                bool MemberSelectors::DataMemberSelector::select(const Member &member) const {
                    return !member.isLiteMember();
                }

                void MemberSelectors::DataMemberSelector::toString(std::ostream &os) const {
                    os << "Default DataMemberSelector";
                }

                const std::unique_ptr<MemberSelector> MemberSelectors::DATA_MEMBER_SELECTOR(
                        new MemberSelectors::DataMemberSelector());
            }

            namespace impl {
                VectorClock::VectorClock() = default;

                VectorClock::VectorClock(const VectorClock::TimestampVector &replicaLogicalTimestamps)
                        : replica_timestamp_entries_(replicaLogicalTimestamps) {
                    for (const VectorClock::TimestampVector::value_type &replicaTimestamp : replicaLogicalTimestamps) {
                        replica_timestamps_[replicaTimestamp.first] = replicaTimestamp.second;
                    }
                }

                VectorClock::TimestampVector VectorClock::entrySet() {
                    return replica_timestamp_entries_;
                }

                bool VectorClock::isAfter(VectorClock &other) {
                    bool anyTimestampGreater = false;
                    for (const VectorClock::TimestampMap::value_type &otherEntry : other.replica_timestamps_) {
                        const auto &replicaId = otherEntry.first;
                        int64_t otherReplicaTimestamp = otherEntry.second;
                        std::pair<bool, int64_t> localReplicaTimestamp = getTimestampForReplica(replicaId);

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

                std::pair<bool, int64_t> VectorClock::getTimestampForReplica(boost::uuids::uuid replicaId) {
                    if (replica_timestamps_.count(replicaId) == 0) {
                        return std::make_pair(false, -1);
                    }
                    return std::make_pair(true, replica_timestamps_[replicaId]);
                }
            }
        }

        namespace internal {
            namespace partition {
                namespace strategy {
                    std::string StringPartitioningStrategy::getBaseName(const std::string &name) {
                        size_t indexOf = name.find('@');
                        if (indexOf == std::string::npos) {
                            return name;
                        }
                        return name.substr(0, indexOf);
                    }

                    std::string StringPartitioningStrategy::getPartitionKey(const std::string &key) {
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
        return boost::hash<boost::uuids::uuid>()(k.getUuid());
    }
}

