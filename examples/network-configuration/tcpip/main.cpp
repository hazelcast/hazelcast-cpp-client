/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

int main() {
    const char *serverIp = "127.0.0.1";
    const int port = 5701;
    hazelcast::client::client_config config;
    config.get_network_config().add_address({serverIp, port}).add_addresses({{"127.0.0.1", 5702},
                                                                            {"192.168.1.10", 5701}});

    hazelcast::client::hazelcast_client hz(std::move(config));

    auto map = hz.get_map("test map");

    std::cout << "Finished" << std::endl;

    return 0;
}

