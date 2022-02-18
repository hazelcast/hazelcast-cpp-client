/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include "ClientTest.h"

#include <chrono>
#include <thread>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <hazelcast/client/client_config.h>
#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/member.h>

#include "HazelcastServerFactory.h"
#include "remote_controller_client.h"

namespace hazelcast {
namespace client {
namespace test {

ClientTest::ClientTest()
{
    logger_ = std::make_shared<logger>(
      "Test", get_test_name(), logger::level::info, logger::default_handler);

    remote_controller_client().ping();
}

logger&
ClientTest::get_logger()
{
    return *logger_;
}

std::string
ClientTest::get_test_name()
{
    const auto* info = testing::UnitTest::GetInstance()->current_test_info();
    std::ostringstream out;
    out << info->test_case_name() << "_" << info->name();
    return out.str();
}

std::string
ClientTest::get_ca_file_path()
{
    return "hazelcast/test/resources/cpp_client.crt";
}

client_config
ClientTest::get_config(bool ssl_enabled, bool smart)
{
    client_config config;

#ifdef HZ_BUILD_WITH_SSL
    if (ssl_enabled) {
        config.set_cluster_name(get_ssl_cluster_name());
        boost::asio::ssl::context ctx(
          boost::asio::ssl::context::method::tlsv12_client);
        ctx.set_default_verify_paths();
        ctx.load_verify_file(get_ca_file_path());

        config.get_network_config().get_ssl_config().set_context(
          std::move(ctx));
    }
#endif

    config.get_connection_strategy_config()
      .get_retry_config()
      .set_cluster_connect_timeout(std::chrono::seconds(120));
    config.get_network_config().set_smart_routing(smart);

    return config;
}

hazelcast_client
ClientTest::get_new_client()
{
    return hazelcast::new_client(get_config()).get();
}

std::string
ClientTest::get_ssl_file_path()
{
    return "hazelcast/test/resources/hazelcast-ssl.xml";
}

std::string
ClientTest::random_map_name()
{
    return random_string();
}

std::string
ClientTest::random_string()
{
    // performance is not important, hence we can use random_device for the
    // tests
    std::random_device rand{};
    return boost::uuids::to_string(
      boost::uuids::basic_random_generator<std::random_device>{ rand }());
}

boost::uuids::uuid
ClientTest::generate_key_owned_by(spi::ClientContext& context,
                                  const member& member)
{
    spi::impl::ClientPartitionServiceImpl& partitionService =
      context.get_partition_service();
    serialization::pimpl::SerializationService& serializationService =
      context.get_serialization_service();
    while (true) {
        auto id = context.random_uuid();
        int partitionId =
          partitionService.get_partition_id(serializationService.to_data(id));
        std::shared_ptr<impl::Partition> partition =
          partitionService.get_partition(partitionId);
        auto owner = partition->get_owner();
        if (owner && *owner == member) {
            return id;
        }
    }
}

std::string
ClientTest::get_ssl_cluster_name()
{
    return "ssl-dev";
}

HazelcastServerFactory&
ClientTest::default_server_factory()
{
    static HazelcastServerFactory factory(
      "hazelcast/test/resources/hazelcast.xml");

    return factory;
}

} // namespace test
} // namespace client
} // namespace hazelcast
