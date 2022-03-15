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
#include <hazelcast/client/hazelcast_client.h>

int main(int argc, char **argv) {
    constexpr const char *USAGE = "USAGE: ssl-connect-cloud <Cluster group name> <Cluster discovery token> <ca file path> <client certificate> <client key file> <client key PEM pass phrase>";
    if (argc != 7) {
        std::cerr << USAGE << std::endl;
        return -1;
    }

    std::string cluster_name = argv[1];
    std::string cloud_token = argv[2];
    std::string ca_file_path = argv[3];
    std::string client_certificate = argv[4];
    std::string client_key = argv[5];
    std::string client_key_pem_pass_phrase = argv[6];

    hazelcast::client::client_config config;
    config.set_cluster_name(cluster_name);
    auto &cloud_configuration = config.get_network_config().get_cloud_config();
    cloud_configuration.enabled = true;
    cloud_configuration.discovery_token = cloud_token;

    // ssl configuration
    boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12);
    ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    ctx.load_verify_file(ca_file_path);
    ctx.use_certificate_file(client_certificate, boost::asio::ssl::context::pem);
    ctx.set_password_callback([&] (std::size_t max_length, boost::asio::ssl::context::password_purpose purpose) {
        return client_key_pem_pass_phrase;
    });
    ctx.use_private_key_file(client_key, boost::asio::ssl::context::pem);
    config.get_network_config().get_ssl_config().set_context(std::move(ctx));

    auto hz = hazelcast::new_client(std::move(config)).get();

    auto map = hz.get_map("MyMap").get();

    map->put(1, 100).get();
    map->put(2, 200).get();

    auto value = map->get<int, int>(1).get();

    if (value) {
        std::cout << "Value for key 1 is " << value.value() << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
