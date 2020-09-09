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
        Cluster::Cluster(spi::impl::ClientClusterServiceImpl &clusterService)
                : clusterService(clusterService) {
        }

        std::vector<Member> Cluster::getMembers() {
            return clusterService.getMemberList();
        }

        boost::uuids::uuid Cluster::addMembershipListener(MembershipListener &&listener) {
            return clusterService.addMembershipListener(std::move(listener));
        }

        bool Cluster::removeMembershipListener(boost::uuids::uuid registrationId) {
            return clusterService.removeMembershipListener(registrationId);
        }

        Member::Member() : liteMember(false) {
        }

        Member::Member(Address address, boost::uuids::uuid uuid, bool lite, std::unordered_map<std::string, std::string> attr) :
                address(address), uuid(uuid), liteMember(lite), attributes(attr) {
        }

        Member::Member(Address memberAddress) : address(memberAddress), liteMember(false) {
        }

        Member::Member(boost::uuids::uuid uuid) : uuid(uuid), liteMember(false) {
        }

        bool Member::operator==(const Member &rhs) const {
            return uuid == rhs.uuid;
        }

        const Address &Member::getAddress() const {
            return address;
        }

        boost::uuids::uuid Member::getUuid() const {
            return uuid;
        }

        bool Member::isLiteMember() const {
            return liteMember;
        }

        const std::unordered_map<std::string, std::string> &Member::getAttributes() const {
            return attributes;
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
            std::unordered_map<std::string, std::string>::const_iterator it = attributes.find(key);
            if (attributes.end() != it) {
                return &(it->second);
            } else {
                return NULL;
            }
        }

        bool Member::lookupAttribute(const std::string &key) const {
            return attributes.find(key) != attributes.end();
        }

        bool Member::operator<(const Member &rhs) const {
            return uuid < rhs.uuid;
        }

        Endpoint::Endpoint(boost::uuids::uuid uuid, boost::optional<Address> socketAddress)
                : uuid(uuid), socketAddress(std::move(socketAddress)) {}

        boost::uuids::uuid Endpoint::getUuid() const {
            return uuid;
        }

        const boost::optional<Address> &Endpoint::getSocketAddress() const {
            return socketAddress;
        }

        MembershipEvent::MembershipEvent(Cluster &cluster, const Member &member, MembershipEventType eventType,
                                         const std::unordered_map<boost::uuids::uuid, Member, boost::hash<boost::uuids::uuid>> &membersList) :
                cluster(cluster), member(member), eventType(eventType), members(membersList) {
        }

        MembershipEvent::~MembershipEvent() = default;

        std::unordered_map<boost::uuids::uuid, Member, boost::hash<boost::uuids::uuid>> MembershipEvent::getMembers() const {
            return members;
        }

        const Cluster &MembershipEvent::getCluster() const {
            return cluster;
        }

        MembershipEvent::MembershipEventType MembershipEvent::getEventType() const {
            return eventType;
        }

        const Member &MembershipEvent::getMember() const {
            return member;
        }

        Client::Client(boost::uuids::uuid uuid, boost::optional<Address> socketAddress, std::string name,
                       std::unordered_set<std::string> labels) : Endpoint(uuid, std::move(socketAddress)), name(std::move(name)),
                                                                 labels_(std::move(labels)) {}

        const std::string &Client::getName() const {
            return name;
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
                return members[++index % members.size()];
            }

            RoundRobinLB::RoundRobinLB(const RoundRobinLB &rhs) : index(rhs.index.load()) {
            }

            void RoundRobinLB::operator=(const RoundRobinLB &rhs) {
                index.store(rhs.index.load());
            }

            AbstractLoadBalancer::AbstractLoadBalancer(const AbstractLoadBalancer &rhs) {
                *this = rhs;
            }

            void AbstractLoadBalancer::operator=(const AbstractLoadBalancer &rhs) {
                std::lock_guard<std::mutex> lg(rhs.membersLock);
                std::lock_guard<std::mutex> lg2(membersLock);
                membersRef = rhs.membersRef;
                cluster = rhs.cluster;
            }

            void AbstractLoadBalancer::init(Cluster &cluster) {
                this->cluster = &cluster;
                setMembersRef();

                cluster.addMembershipListener(
                    MembershipListener()
                        .on_init([this](const InitialMembershipEvent &){
                            setMembersRef();
                        })
                        .on_added([this](const MembershipEvent &){
                            setMembersRef();
                        })
                        .on_removed([this](const MembershipEvent &){
                            setMembersRef();
                        })
                );
            }

            void AbstractLoadBalancer::setMembersRef() {
                std::lock_guard<std::mutex> lg(membersLock);
                membersRef = cluster->getMembers();
            }

            std::vector<Member> AbstractLoadBalancer::getMembers() {
                std::lock_guard<std::mutex> lg(membersLock);
                return membersRef;
            }

            AbstractLoadBalancer::~AbstractLoadBalancer() = default;

            AbstractLoadBalancer::AbstractLoadBalancer() : cluster(NULL) {
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
                        : replicaTimestampEntries(replicaLogicalTimestamps) {
                    for (const VectorClock::TimestampVector::value_type &replicaTimestamp : replicaLogicalTimestamps) {
                        replicaTimestamps[replicaTimestamp.first] = replicaTimestamp.second;
                    }
                }

                VectorClock::TimestampVector VectorClock::entrySet() {
                    return replicaTimestampEntries;
                }

                bool VectorClock::isAfter(VectorClock &other) {
                    bool anyTimestampGreater = false;
                    for (const VectorClock::TimestampMap::value_type &otherEntry : other.replicaTimestamps) {
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
                    return anyTimestampGreater || other.replicaTimestamps.size() < replicaTimestamps.size();
                }

                std::pair<bool, int64_t> VectorClock::getTimestampForReplica(boost::uuids::uuid replicaId) {
                    if (replicaTimestamps.count(replicaId) == 0) {
                        return std::make_pair(false, -1);
                    }
                    return std::make_pair(true, replicaTimestamps[replicaId]);
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

