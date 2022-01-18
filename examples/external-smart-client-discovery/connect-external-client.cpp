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
#include <hazelcast/client/hazelcast_client.h>

int
main(int argc, char** argv)
{
    hazelcast::client::client_config config;
    constexpr int server_port = 5701;
    constexpr const char* server_public_address = "myserver.publicaddress.com";
    config.get_network_config().use_public_address(true).add_address(
      hazelcast::client::address{ server_public_address, server_port });
    // The client will try to connect to the cluster initially using the
    // provided server public address and it will connect to the other members
    // of the cluster using their public interfaces if available.
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
