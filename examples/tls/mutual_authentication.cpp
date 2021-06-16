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
/**
 * You need to provide compile flag -DHZ_BUILD_WITH_SSL when compiling.
 */
#include <hazelcast/client/hazelcast_client.h>

int
main()
{
    hazelcast::client::client_config config;
    hazelcast::client::address serverAddress("127.0.0.1", 5701);
    config.get_network_config().add_address(serverAddress);

    boost::asio::ssl::context ctx(boost::asio::ssl::context::method::tlsv12_client);
    ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    ctx.set_default_verify_paths();

    // This config is to validate the server certificate if server does not have a CA signed
    // certificate
    ctx.load_verify_file("/path/to/my/server/public/certificate");

    // The following two lines configure the client to use the client certificate to introduce
    // itself to the server
    ctx.use_certificate_file("/path/to/certificate", boost::asio::ssl::context::pem);
    ctx.use_private_key_file("/path/to/private_key", boost::asio::ssl::context::pem);

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
