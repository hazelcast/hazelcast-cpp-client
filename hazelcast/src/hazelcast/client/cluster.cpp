/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/cluster.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/membership_listener.h"
#include "hazelcast/client/initial_membership_event.h"
#include "hazelcast/client/member.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/membership_event.h"
#include "hazelcast/client/impl/vector_clock.h"
#include "hazelcast/client/member_selectors.h"
#include "hazelcast/client/internal/partition/strategy/StringPartitioningStrategy.h"

namespace hazelcast {
namespace client {
cluster::cluster(spi::impl::ClientClusterServiceImpl& cluster_service)
  : cluster_service_(cluster_service)
{}

std::vector<member>
cluster::get_members()
{
    return cluster_service_.get_member_list();
}

boost::uuids::uuid
cluster::add_membership_listener(membership_listener&& listener)
{
    return cluster_service_.add_membership_listener(std::move(listener));
}

bool
cluster::remove_membership_listener(boost::uuids::uuid registration_id)
{
    return cluster_service_.remove_membership_listener(registration_id);
}

member::member()
  : lite_member_(false)
  , version_{ 0, 0, 0 }
{}

member::member(address member_address,
               boost::uuids::uuid uuid,
               bool lite,
               std::unordered_map<std::string, std::string> attr,
               std::unordered_map<endpoint_qualifier, address> address_map,
               version v)
  : address_(std::move(member_address))
  , uuid_(uuid)
  , lite_member_(lite)
  , attributes_(std::move(attr))
  , address_map_(std::move(address_map))
  , version_(v)
{}

member::member(address member_address)
  : address_(member_address)
  , lite_member_(false)
  , version_{ 0, 0, 0 }
{}

member::member(boost::uuids::uuid uuid)
  : uuid_(uuid)
  , lite_member_(false)
  , version_{ 0, 0, 0 }
{}

const address&
member::get_address() const
{
    return address_;
}

boost::uuids::uuid
member::get_uuid() const
{
    return uuid_;
}

bool
member::is_lite_member() const
{
    return lite_member_;
}

const std::unordered_map<std::string, std::string>&
member::get_attributes() const
{
    return attributes_;
}

std::ostream&
operator<<(std::ostream& out, const member& member)
{
    const address& address = member.get_address();
    out << "Member[";
    out << address.get_host();
    out << "]";
    out << ":";
    out << address.get_port();
    out << " - " << boost::uuids::to_string(member.get_uuid());
    if (member.is_lite_member()) {
        out << " lite";
    }
    return out;
}

const std::string*
member::get_attribute(const std::string& key) const
{
    std::unordered_map<std::string, std::string>::const_iterator it =
      attributes_.find(key);
    if (attributes_.end() != it) {
        return &(it->second);
    } else {
        return NULL;
    }
}

member::version
member::get_version() const
{
    return version_;
}

bool
member::lookup_attribute(const std::string& key) const
{
    return attributes_.find(key) != attributes_.end();
}

bool
member::operator<(const member& rhs) const
{
    return uuid_ < rhs.uuid_;
}

const std::unordered_map<endpoint_qualifier, address>&
member::address_map() const
{
    return address_map_;
}

bool
operator==(const member& lhs, const member& rhs)
{
    return lhs.address_ == rhs.address_ && lhs.uuid_ == rhs.uuid_;
}

endpoint::endpoint(boost::uuids::uuid uuid,
                   boost::optional<address> socket_address)
  : uuid_(uuid)
  , socket_address_(std::move(socket_address))
{}

boost::uuids::uuid
endpoint::get_uuid() const
{
    return uuid_;
}

const boost::optional<address>&
endpoint::get_socket_address() const
{
    return socket_address_;
}

membership_event::membership_event(
  cluster& cluster,
  const member& m,
  membership_event_type event_type,
  const std::unordered_map<boost::uuids::uuid,
                           member,
                           boost::hash<boost::uuids::uuid>>& members_list)
  : cluster_(cluster)
  , member_(m)
  , event_type_(event_type)
  , members_(members_list)
{}

membership_event::~membership_event() = default;

std::unordered_map<boost::uuids::uuid, member, boost::hash<boost::uuids::uuid>>
membership_event::get_members() const
{
    return members_;
}

cluster&
membership_event::get_cluster()
{
    return cluster_;
}

membership_event::membership_event_type
membership_event::get_event_type() const
{
    return event_type_;
}

const member&
membership_event::get_member() const
{
    return member_;
}

local_endpoint::local_endpoint(boost::uuids::uuid uuid,
                               boost::optional<address> socket_address,
                               std::string name,
                               std::unordered_set<std::string> labels)
  : endpoint(uuid, std::move(socket_address))
  , name_(std::move(name))
  , labels_(std::move(labels))
{}

const std::string&
local_endpoint::get_name() const
{
    return name_;
}

namespace impl {
vector_clock::vector_clock() = default;

vector_clock::vector_clock(
  const vector_clock::timestamp_vector& replica_logical_timestamps)
  : replica_timestamp_entries_(replica_logical_timestamps)
{
    for (const vector_clock::timestamp_vector::value_type& replicaTimestamp :
         replica_logical_timestamps) {
        replica_timestamps_[replicaTimestamp.first] = replicaTimestamp.second;
    }
}

vector_clock::timestamp_vector
vector_clock::entry_set()
{
    return replica_timestamp_entries_;
}

bool
vector_clock::is_after(vector_clock& other)
{
    bool anyTimestampGreater = false;
    for (const vector_clock::timestamp_map::value_type& otherEntry :
         other.replica_timestamps_) {
        const auto& replicaId = otherEntry.first;
        int64_t otherReplicaTimestamp = otherEntry.second;
        std::pair<bool, int64_t> localReplicaTimestamp =
          get_timestamp_for_replica(replicaId);

        if (!localReplicaTimestamp.first ||
            localReplicaTimestamp.second < otherReplicaTimestamp) {
            return false;
        } else if (localReplicaTimestamp.second > otherReplicaTimestamp) {
            anyTimestampGreater = true;
        }
    }
    // there is at least one local timestamp greater or local vector clock has
    // additional timestamps
    return anyTimestampGreater ||
           other.replica_timestamps_.size() < replica_timestamps_.size();
}

std::pair<bool, int64_t>
vector_clock::get_timestamp_for_replica(boost::uuids::uuid replica_id)
{
    if (replica_timestamps_.count(replica_id) == 0) {
        return std::make_pair(false, -1);
    }
    return std::make_pair(true, replica_timestamps_[replica_id]);
}
} // namespace impl

bool
member_selectors::data_member_selector::select(const member& member) const
{
    return !member.is_lite_member();
}

const std::unique_ptr<member_selector> member_selectors::DATA_MEMBER_SELECTOR(
  new member_selectors::data_member_selector());

namespace internal {
namespace partition {
namespace strategy {
std::string
StringPartitioningStrategy::get_base_name(const std::string& name)
{
    size_t index_of = name.find('@');
    if (index_of == std::string::npos) {
        return name;
    }
    return name.substr(0, index_of);
}

std::string
StringPartitioningStrategy::get_partition_key(const std::string& key)
{
    size_t firstIndexOf = key.find('@');
    if (firstIndexOf == std::string::npos) {
        return key;
    } else {
        return key.substr(firstIndexOf + 1);
    }
}
} // namespace strategy
} // namespace partition
} // namespace internal

bool
operator==(const endpoint_qualifier& lhs, const endpoint_qualifier& rhs)
{
    return lhs.type == rhs.type && lhs.identifier == rhs.identifier;
}

bool
member::version::operator==(const member::version& rhs) const
{
    return major == rhs.major && minor == rhs.minor && patch == rhs.patch;
}

bool
member::version::operator!=(const member::version& rhs) const
{
    return !(rhs == *this);
}

bool
member::version::operator<(const member::version& rhs) const
{
    if (major < rhs.major)
        return true;
    if (rhs.major < major)
        return false;
    if (minor < rhs.minor)
        return true;
    if (rhs.minor < minor)
        return false;
    return patch < rhs.patch;
}

bool
member::version::operator>(const member::version& rhs) const
{
    return rhs < *this;
}

bool
member::version::operator<=(const member::version& rhs) const
{
    return !(rhs < *this);
}

bool
member::version::operator>=(const member::version& rhs) const
{
    return !(*this < rhs);
}

std::ostream&
operator<<(std::ostream& os, const member::version& version)
{
    os << version.major << "." << version.minor << "." << version.patch;
    return os;
}
} // namespace client
} // namespace hazelcast

namespace std {
std::size_t
hash<hazelcast::client::member>::operator()(
  const hazelcast::client::member& m) const noexcept
{
    std::size_t seed = 0;
    boost::hash_combine(
      seed, std::hash<hazelcast::client::address>()(m.get_address()));
    boost::hash_combine(seed, m.get_uuid());
    return seed;
}

std::size_t
hash<hazelcast::client::endpoint_qualifier>::operator()(
  const hazelcast::client::endpoint_qualifier& e) const noexcept
{
    std::size_t seed = 0;
    boost::hash_combine(seed, e.type);
    boost::hash_combine(seed, e.identifier);
    return seed;
}
} // namespace std
