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

#include <boost/uuid/uuid.hpp>

#include <gtest/gtest.h>

#include <hazelcast/client/client_config.h>
#include <hazelcast/logger.h>

namespace hazelcast {
namespace client {

class member;

namespace spi {
class ClientContext;
}

namespace test {

class HazelcastServerFactory;

class ClientTest : public ::testing::Test
{
public:
    ClientTest();

    logger& get_logger();

    static std::string get_test_name();
    static std::string get_ca_file_path();
    static std::string random_map_name();
    static std::string random_string();
    static boost::uuids::uuid generate_key_owned_by(spi::ClientContext& context,
                                                    const member& member);
    static client_config get_config(bool ssl_enabled = false, bool smart = true);
    static std::string get_ssl_cluster_name();
    static hazelcast_client get_new_client();
    static std::string get_ssl_file_path();
    static HazelcastServerFactory& default_server_factory();

private:
    std::shared_ptr<logger> logger_;
};

} // namespace test
} // namespace client
} // namespace hazelcast
