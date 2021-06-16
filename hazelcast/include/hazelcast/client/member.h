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

#include <unordered_map>
#include <functional>
#include <memory>

#include "hazelcast/client/address.h"
#include <boost/uuid/uuid.hpp>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {

/**
 * hz_cluster member class. The default implementation
 *
 * @see Cluster
 * @see MembershipListener
 */
class HAZELCAST_API member
{
public:
    /**
     * PUT even type representing an addition of an attribute
     * REMOVE event type representing a deletion of an attribute
     */
    enum member_attribute_operation_type
    {
        PUT = 1,
        REMOVE = 2
    };

    member();

    member(address address,
           boost::uuids::uuid uuid,
           bool lite,
           std::unordered_map<std::string, std::string> attr);

    member(address member_address);

    member(boost::uuids::uuid uuid);

    /**
     * comparison operation
     */
    bool operator==(const member&) const;

    /**
     *
     * Lite member is does not hold data.
     * @return true if member is lite.
     */
    bool is_lite_member() const;

    /**
     * Returns the socket address of this member.
     *
     * @return socket address of this member
     */
    const address& get_address() const;

    /**
     * Returns UUID of this member.
     *
     * @return UUID of this member.
     */
    boost::uuids::uuid get_uuid() const;

    const std::unordered_map<std::string, std::string>& get_attributes() const;

    /**
     * Returns the value of the specified key for this member or
     * default constructed value if value is undefined.
     *
     * @tparam AttributeType type template for attribute type
     * @param key The key to lookup.
     * @return The value for this members key.
     */
    const std::string* get_attribute(const std::string& key) const;

    /**
     * check if an attribute is defined for given key.
     *
     * @tparam key for the attribute
     * @return true if attribute is defined.
     */
    bool lookup_attribute(const std::string& key) const;

    bool operator<(const member& rhs) const;

private:
    address address_;
    boost::uuids::uuid uuid_;
    bool lite_member_;
    std::unordered_map<std::string, std::string> attributes_;
};

std::ostream HAZELCAST_API&
operator<<(std::ostream& out, const member& member);
} // namespace client
} // namespace hazelcast

namespace std {
template<>
struct HAZELCAST_API hash<hazelcast::client::member>
{
    std::size_t operator()(const hazelcast::client::member& k) const noexcept;
};
} // namespace std

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
