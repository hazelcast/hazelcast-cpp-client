/*
 * Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.
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

#include <gtest/gtest.h>
#include <hazelcast/client/client_config.h>
#include <hazelcast/client/client_properties.h>

using namespace hazelcast::client;

class IoThreadCountPropertyTest : public ::testing::Test {};

TEST_F(IoThreadCountPropertyTest, default_value_is_three)
{
    std::unordered_map<std::string, std::string> empty_props;
    client_properties props(empty_props);
    auto value = props.get_integer(props.get_io_thread_count());
    ASSERT_EQ(3, value);
}

TEST_F(IoThreadCountPropertyTest, default_string_constant)
{
    ASSERT_EQ("3", client_properties::IO_THREAD_COUNT_DEFAULT);
}

TEST_F(IoThreadCountPropertyTest, property_name_constant)
{
    ASSERT_EQ("hazelcast.client.io.thread.count",
              client_properties::IO_THREAD_COUNT);
}

TEST_F(IoThreadCountPropertyTest, custom_value_via_set_property)
{
    std::unordered_map<std::string, std::string> props;
    props[client_properties::IO_THREAD_COUNT] = "5";
    client_properties cp(props);
    auto value = cp.get_integer(cp.get_io_thread_count());
    ASSERT_EQ(5, value);
}

TEST_F(IoThreadCountPropertyTest, single_io_thread)
{
    std::unordered_map<std::string, std::string> props;
    props[client_properties::IO_THREAD_COUNT] = "1";
    client_properties cp(props);
    auto value = cp.get_integer(cp.get_io_thread_count());
    ASSERT_EQ(1, value);
}

TEST_F(IoThreadCountPropertyTest, set_via_client_config)
{
    client_config config;
    config.set_property(client_properties::IO_THREAD_COUNT, "7");
    auto& stored = config.get_properties();
    auto it = stored.find(client_properties::IO_THREAD_COUNT);
    ASSERT_NE(it, stored.end());
    ASSERT_EQ("7", it->second);
}

// ── Response Thread Count Property Tests ─────────────────────────────────

class ResponseThreadCountPropertyTest : public ::testing::Test {};

TEST_F(ResponseThreadCountPropertyTest, default_value_is_two)
{
    std::unordered_map<std::string, std::string> empty_props;
    client_properties props(empty_props);
    auto value = props.get_integer(props.get_response_thread_count());
    ASSERT_EQ(2, value);
}

TEST_F(ResponseThreadCountPropertyTest, default_string_constant)
{
    ASSERT_EQ("2", client_properties::RESPONSE_THREAD_COUNT_DEFAULT);
}

TEST_F(ResponseThreadCountPropertyTest, property_name_constant)
{
    ASSERT_EQ("hazelcast.client.response.thread.count",
              client_properties::RESPONSE_THREAD_COUNT);
}

TEST_F(ResponseThreadCountPropertyTest, custom_value_via_set_property)
{
    std::unordered_map<std::string, std::string> props;
    props[client_properties::RESPONSE_THREAD_COUNT] = "4";
    client_properties cp(props);
    auto value = cp.get_integer(cp.get_response_thread_count());
    ASSERT_EQ(4, value);
}

TEST_F(ResponseThreadCountPropertyTest, set_via_client_config)
{
    client_config config;
    config.set_property(client_properties::RESPONSE_THREAD_COUNT, "8");
    auto& stored = config.get_properties();
    auto it = stored.find(client_properties::RESPONSE_THREAD_COUNT);
    ASSERT_NE(it, stored.end());
    ASSERT_EQ("8", it->second);
}
