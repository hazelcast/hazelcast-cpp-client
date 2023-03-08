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

#include <iosfwd>
#include <vector>

#include "hazelcast/client/address.h"
#include "hazelcast/util/export.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
class logger;

namespace util {
/**
 * Holds address
 */
class HAZELCAST_API AddressHolder
{
public:
    AddressHolder(const std::string& address,
                  const std::string& scope_id,
                  int port);

    const std::string& get_address() const;

    const std::string& get_scope_id() const;

    int get_port() const;

    friend std::ostream& operator<<(std::ostream& os,
                                    const AddressHolder& holder);

private:
    std::string address_;
    std::string scope_id_;
    int port_;
};

/**
 * This is a client side utility class for working with addresses and cluster
 * connections
 */
class HAZELCAST_API AddressHelper
{
public:
    static std::vector<client::address> get_socket_addresses(
      const std::string& address,
      logger& lg);

    static std::string get_scoped_hostname(const AddressHolder& addressHolder);

private:
    static const int MAX_PORT_TRIES;
    static const int INITIAL_FIRST_PORT;

    static std::vector<client::address> get_possible_socket_addresses(
      int port,
      const std::string& scoped_address,
      int port_try_count,
      logger& lg);
};
} // namespace util
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
