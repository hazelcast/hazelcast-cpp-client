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

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <crtdefs.h>
#endif

#include "cpp-controller/RemoteController.h"

#include <memory>
#include <ostream>

#include <hazelcast/client/address.h>
#include <hazelcast/logger.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace hazelcast::client::test::remote;

namespace hazelcast {
namespace client {
namespace test {

class HazelcastServerFactory
{
public:
    HazelcastServerFactory(const std::string& server_xml_config_file_path);

    HazelcastServerFactory(const std::string& server_address,
                           const std::string& server_xml_config_file_path);

    const std::string& get_server_address();

    remote::Member start_server();

    bool shutdown_server(const remote::Member& member);

    bool terminate_server(const remote::Member& member);

    const std::string& get_cluster_id() const;

    ~HazelcastServerFactory();

private:
    std::shared_ptr<logger> logger_;
    std::string server_address_;
    std::string cluster_id_;

    std::string read_from_xml_file(const std::string& xml_file_path);
};
} // namespace test
} // namespace client
} // namespace hazelcast
