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

#include <utility>
#include <iterator>
#include <functional>
#include <boost/uuid/uuid.hpp>

#include <gtest/gtest.h>

#include <hazelcast/client/client_config.h>
#include <hazelcast/client/imap.h>
#include <hazelcast/logger.h>

#include "remote_controller_client.h"

namespace hazelcast {
namespace client {

class member;

namespace spi {
class ClientContext;
}

namespace test {

template<typename T, typename = void>
struct generator;
struct key_int_generator;

class HazelcastServerFactory;

class ClientTest : public ::testing::Test
{
public:
    using imap_t = std::shared_ptr<hazelcast::client::imap>;

    ClientTest();

    logger& get_logger();

    static std::string get_test_name();
    static std::string get_ca_file_path();
    static std::string random_map_name();
    static boost::uuids::uuid generate_key_owned_by(spi::ClientContext& context,
                                                    const member& member);
    static client_config get_config(bool ssl_enabled = false,
                                    bool smart = true);
    static std::string get_ssl_cluster_name();
    static hazelcast_client get_new_client();
    static std::string get_ssl_file_path();
    static HazelcastServerFactory& default_server_factory();

    template<typename Value = int,
             typename Key = int,
             typename ValueGenerator = generator<Value>,
             typename KeyGenerator = key_int_generator>
    std::unordered_map<Key, Value> populate_map(
      imap_t map,
      int n_entries = 10,
      ValueGenerator&& value_gen = ValueGenerator{},
      KeyGenerator&& key_gen = KeyGenerator{})
    {
        std::unordered_map<Key, Value> entries;
        entries.reserve(n_entries);

        generate_n(
          inserter(entries, end(entries)), n_entries, [&value_gen, &key_gen]() {
              return std::make_pair(key_gen(), value_gen());
          });

        for (const auto& p : entries) {
            map->put(p.first, p.second).get();
        }

        return entries;
    }

    template<typename Value = int,
             typename Key = int,
             typename ValueGenerator = generator<Value>,
             typename KeyGenerator = key_int_generator>
    std::unordered_map<Key, Value> populate_map_via_rc(
      std::function<std::string(const Value&)> fn,
      const std::string& map_name,
      const std::string& cluster_id,
      int n_entries = 10,
      ValueGenerator&& value_gen = ValueGenerator{},
      KeyGenerator&& key_gen = KeyGenerator{})
    {
        std::unordered_map<Key, Value> entries;
        entries.reserve(n_entries);

        generate_n(
          inserter(entries, end(entries)), n_entries, [&value_gen, &key_gen]() {
              return std::make_pair(key_gen(), value_gen());
          });

        for (std::pair<Key, Value> p : entries) {
            remote::Response response;

            remote_controller_client().executeOnController(
              response,
              cluster_id,
              (boost::format(
                 R"(
                    var map = instance_0.getMap("%1%");
                    map.set(new java.lang.Integer(%2%), %3%);

                    result = "" + true;
                )") %
               map_name % std::to_string(p.first) % fn(p.second))
                .str(),
              remote::Lang::JAVASCRIPT);

            if (response.result != "true")
                throw std::logic_error{ response.message };
        }

        return entries;
    }

private:
    std::shared_ptr<logger> logger_;
};

template<typename T>
struct generator<T, typename std::enable_if<std::is_integral<T>::value>::type>
{
    T operator()() { return T(rand()); }
};

struct key_int_generator
{
    int x{};

    int operator()() { return x++; }
};

template<>
struct generator<std::string>
{
    std::string operator()() { return ClientTest::random_map_name(); }
};

template<>
struct generator<hazelcast_json_value>
{
    hazelcast_json_value operator()()
    {
        generator<std::string> str_gen;
        generator<int> int_gen;

        return hazelcast_json_value{ (boost::format(
                                        R"({
                        "name" : "%1%" ,
                        "value" : %2%
                    })") % str_gen() % int_gen())
                                       .str() };
    }
};

template<>
struct generator<local_date>
{
    local_date operator()()
    {
        generator<int> int_gen;

        return local_date{ 2000 + int_gen() % 50,
                           uint8_t(int_gen() % 11 + 1),
                           uint8_t(int_gen() % 27 + 1) };
    }
};

template<>
struct generator<local_time>
{
    local_time operator()()
    {
        generator<int> int_gen;

        return local_time{ uint8_t(int_gen() % 24),
                           uint8_t(int_gen() % 60),
                           uint8_t(int_gen() % 60),
                           int_gen() % 123456 };
    }
};

template<>
struct generator<local_date_time>
{
    local_date_time operator()()
    {
        generator<local_date> date_gen;
        generator<local_time> time_gen;

        return local_date_time{ date_gen(), time_gen() };
    }
};

template<>
struct generator<offset_date_time>
{
    offset_date_time operator()()
    {
        generator<int> int_gen;
        generator<local_date_time> date_time_gen;

        return offset_date_time{ date_time_gen(), int_gen() % 12 * 3600 };
    }
};

} // namespace test
} // namespace client
} // namespace hazelcast
