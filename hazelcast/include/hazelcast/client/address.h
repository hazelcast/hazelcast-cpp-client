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
#pragma once

#include "hazelcast/util/byte.h"
#include "hazelcast/util/export.h"
#include "hazelcast/client/serialization/serialization.h"

#include <string>
#include <iosfwd>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {

/**
 * Represents an address of a client or member in the cluster.
 */
class HAZELCAST_API address
{
    friend struct serialization::hz_serializer<address>;
    friend struct std::hash<hazelcast::client::address>;

public:
    address(std::string hostname, int port, unsigned long scope_id);

    static constexpr int ID = 0;

    /**
     * Constructor
     */
    address();

    address(std::string url, int port);

    /**
     * @param address to be compared.
     */
    bool operator==(const address& address) const;

    /**
     * @param address to be compared.
     */
    bool operator!=(const address& address) const;

    /**
     * @return port number.
     */
    int get_port() const;

    /**
     *
     * @return true if the address is ip V4 address, false otherwise.
     */
    bool is_ip_v4() const;

    /**
     * @return host address as string
     */
    const std::string& get_host() const;

    unsigned long get_scope_id() const;

    bool operator<(const address& rhs) const;

    std::string to_string() const;

private:
    std::string host_;
    int port_;
    byte type_;
    unsigned long scope_id_;

    static constexpr byte IPV4 = 4;
    static constexpr byte IPV6 = 6;
};

namespace serialization {
template<>
struct hz_serializer<address> : public identified_data_serializer
{
    static constexpr int32_t F_ID = 0;
    static constexpr int32_t ADDRESS = 1;
    static int32_t get_factory_id();
    static int32_t get_class_id();
    static void write_data(const address& object, object_data_output& out);
    static address read_data(object_data_input& in);
};
} // namespace serialization

std::ostream HAZELCAST_API&
operator<<(std::ostream& stream, const address& address);
} // namespace client
} // namespace hazelcast

namespace std {
template<>
struct HAZELCAST_API hash<hazelcast::client::address>
{
    std::size_t operator()(
      const hazelcast::client::address& address) const noexcept;
};
} // namespace std

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
